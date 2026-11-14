#include "input/input.h"
#include "input/backends/input_backend.h"
#include "utils/c_array_util.h"
#include "utils/log.h"

#include <string.h>

#ifdef ENABLE_NULL_INPUT_BACKEND
#include "input/backends/null/null_backend.h"
#endif

#define MAX_AVAILABLE_BACKENDS 8

typedef void (*input_backend_init)(input_backend *backend);

// All built-in backends, most preferred first.
static input_backend_init all_backends[] = {
#ifdef ENABLE_NULL_INPUT_BACKEND
    null_input_backend_set_callbacks,
#endif
};
static int all_backends_count = N_ELEMENTS(all_backends);

// Backends present on this platform -- filled by input_scan_backends.
static struct available_backend {
    input_backend_init set_callbacks;
    const char *name;
    const char *description;
} available_backends[MAX_AVAILABLE_BACKENDS];
static int input_backend_count = 0;

static input_backend current_backend;

void input_scan_backends(void) {
    input_backend tmp;
    input_backend_count = 0;
    for(int i = 0; i < all_backends_count; i++) {
        all_backends[i](&tmp);
        if(input_backend_count >= MAX_AVAILABLE_BACKENDS) {
            break;
        }
        if(tmp.is_available()) {
            available_backends[input_backend_count].set_callbacks = all_backends[i];
            available_backends[input_backend_count].name = tmp.get_name();
            available_backends[input_backend_count].description = tmp.get_description();
            log_debug("Input backend '%s' is available", available_backends[input_backend_count].name);
            input_backend_count++;
        }
    }
}

bool input_get_backend_info(const int index, const char **name, const char **description) {
    if(index < 0 || index >= input_backend_count) {
        return false;
    }
    if(name != NULL) {
        *name = available_backends[index].name;
    }
    if(description != NULL) {
        *description = available_backends[index].description;
    }
    return true;
}

int input_get_backend_count(void) {
    return input_backend_count;
}

static bool hunt_backend_by_name(const char *try_name) {
    for(int i = 0; i < input_backend_count; i++) {
        if(strcmp(available_backends[i].name, try_name) != 0) {
            continue;
        }
        available_backends[i].set_callbacks(&current_backend);
        return true;
    }
    return false;
}

static bool find_best_backend(void) {
    if(input_backend_count > 0) {
        available_backends[0].set_callbacks(&current_backend);
        return true;
    }
    return false;
}

static bool input_find_backend(const char *try_name) {
    if(try_name != NULL && strlen(try_name) > 0) {
        if(hunt_backend_by_name(try_name)) {
            log_info("Found configured input backend '%s'!", current_backend.get_name());
            return true;
        }
        log_error("Unable to find specified input backend '%s', trying other alternatives ...", try_name);
    }
    if(find_best_backend()) {
        log_info("Found available input backend '%s'!", current_backend.get_name());
        return true;
    }
    log_error("Unable to find any available input backend!");
    memset(&current_backend, 0, sizeof(input_backend));
    return false;
}

bool input_init(const char *try_name) {
    if(!input_find_backend(try_name)) {
        goto exit_0;
    }
    current_backend.create(&current_backend);
    if(!current_backend.setup_context(current_backend.ctx)) {
        goto exit_1;
    }
    return true;

exit_1:
    current_backend.destroy(&current_backend);
exit_0:
    return false;
}

void input_close(void) {
    current_backend.close_context(current_backend.ctx);
    current_backend.destroy(&current_backend);
}
