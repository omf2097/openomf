#include "video/opengl/shaders.h"
#include "resources/pathmanager.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/str.h"

#include <stdbool.h>

static void print_log(const char *buffer, long len, const char *header) {
    str log;
    str sub;

    // Make sure the string always ends in one \n
    str_from_buf(&log, buffer, len);
    str_rstrip(&log);
    str_append_c(&log, "\n");

    // Print line by line
    size_t pos = 0, last = 0;
    PERROR("--- %s ---", header);
    while(str_find_next(&log, '\n', &pos)) {
        str_from_slice(&sub, &log, last, pos);
        PERROR("%s", str_c(&sub));
        str_free(&sub);
        last = ++pos;
    }
    PERROR("--- end log ---", header);

    str_free(&log);
}

static void print_shader_log(const GLuint shader, const char *header) {
    if(!glIsShader(shader)) {
        PERROR("Attempted to print logs for shader %d: not a shader", shader);
        return;
    }

    int buffer_size = 0, read_size = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &buffer_size);
    char *buffer_data = omf_calloc(buffer_size, 1);
    glGetShaderInfoLog(shader, buffer_size, &read_size, buffer_data);
    print_log(buffer_data, read_size, header);
    omf_free(buffer_data);
}

static void print_program_log(const GLuint program) {
    if(!glIsProgram(program)) {
        PERROR("Attempted to print logs for program %d: not a program", program);
        return;
    }

    int buffer_size = 0, read_size = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &buffer_size);
    char *buffer_data = omf_calloc(buffer_size, 1);
    glGetProgramInfoLog(program, buffer_size, &read_size, buffer_data);
    print_log(buffer_data, read_size, "program");
    omf_free(buffer_data);
}

static bool load_shader(GLuint program_id, GLenum shader_type, const char *shader_file) {
    str shader_path;
    str shader_source;

    str_from_format(&shader_path, "%s%s", pm_get_local_path(SHADER_PATH), shader_file);
    str_from_file(&shader_source, str_c(&shader_path));
    const char *c_str = str_c(&shader_source);

    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &c_str, NULL);
    glCompileShader(shader);

    str_free(&shader_source);
    str_free(&shader_path);

    GLint status = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if(status != GL_TRUE) {
        PERROR("Compilation error for shader %d (file=%s)", shader, shader_file);
        print_shader_log(shader, shader_file);
        return false;
    }
    DEBUG("Compilation succeeded for shader %d (file=%s)", shader, shader_file);
    glAttachShader(program_id, shader);
    return true;
}

void delete_program(GLuint program_id) {
    GLsizei attached_count = 0;
    glGetProgramiv(program_id, GL_ATTACHED_SHADERS, &attached_count);
    GLuint shaders[attached_count];
    glUseProgram(0);
    glGetAttachedShaders(program_id, attached_count, NULL, shaders);
    for(int i = 0; i < attached_count; i++) {
        DEBUG("Shader %d deleted", shaders[i]);
        glDeleteShader(shaders[i]); // Mark for removal, glDeleteProgram will handle deletion.
    }
    glDeleteProgram(program_id);
    DEBUG("Program %d deleted", program_id);
}

bool create_program(GLuint *program_id, const char *vertex_shader, const char *fragment_shader) {
    GLuint id = glCreateProgram();
    if(!load_shader(id, GL_VERTEX_SHADER, vertex_shader))
        goto error_0;
    if(!load_shader(id, GL_FRAGMENT_SHADER, fragment_shader))
        goto error_0;

    glLinkProgram(id);
    GLint status = GL_TRUE;
    glGetProgramiv(id, GL_LINK_STATUS, &status);
    if(status != GL_TRUE) {
        PERROR("Compilation error for program %d (vert=%s, frag=%s)", id, vertex_shader, fragment_shader);
        print_program_log(id);
        goto error_0;
    }

    *program_id = id;
    DEBUG("Compilation succeeded for program %d (vert=%s, frag=%s)", id, vertex_shader, fragment_shader);
    return true;

error_0:
    delete_program(id);
    return false;
}

void bind_uniform_4fv(GLuint program_id, const char *name, GLfloat *data) {
    GLint ref = glGetUniformLocation(program_id, name);
    if(ref == -1) {
        PERROR("Unable to find uniform '%s'; glGetUniformLocation() returned -1", name);
        return;
    }
    glUniformMatrix4fv(ref, 1, GL_FALSE, data);
}

void bind_uniform_li(GLuint program_id, const char *name, GLuint value) {
    GLint ref = glGetUniformLocation(program_id, name);
    if(ref == -1) {
        PERROR("Unable to find uniform '%s'; glGetUniformLocation() returned -1", name);
        return;
    }
    glUniform1i(ref, value);
}

void bind_uniform_block(GLuint program_id, const char *name, GLuint binding) {
    GLuint ref = glGetUniformBlockIndex(program_id, name);
    if(ref == GL_INVALID_INDEX) {
        PERROR("Unable to find ubo '%s'; glGetUniformBlockIndex() returned GL_INVALID_INDEX", name);
        return;
    }
    glUniformBlockBinding(program_id, 0, ref);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, binding);
}

void activate_program(GLuint program_id) {
    glUseProgram(program_id);
}
