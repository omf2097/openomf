#include "input/backends/null/null_backend.h"
#include "input/backends/input_backend.h"
#include "utils/log.h"

static bool is_available(void) {
    return true;
}

static const char *get_description(void) {
    return "NULL input";
}

static const char *get_name(void) {
    return "NULL";
}

static void create_backend(input_backend *backend) {
}

static void destroy_backend(input_backend *backend) {
}

static bool setup_backend_context(void *ctx) {
    log_info("NULL input backend initialized!");
    return true;
}

static void close_backend_context(void *ctx) {
    log_info("NULL input backend closed!");
}

void null_input_backend_set_callbacks(input_backend *null_backend) {
    null_backend->is_available = is_available;
    null_backend->get_description = get_description;
    null_backend->get_name = get_name;
    null_backend->create = create_backend;
    null_backend->destroy = destroy_backend;
    null_backend->setup_context = setup_backend_context;
    null_backend->close_context = close_backend_context;
}
