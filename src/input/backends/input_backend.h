/**
 * @file input_backend.h
 * @brief Input backend vtable
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef INPUT_BACKEND_H
#define INPUT_BACKEND_H

#include <stdbool.h>

typedef struct input_backend input_backend;

typedef bool (*is_input_backend_available_fn)(void);
typedef const char *(*get_input_backend_description_fn)(void);
typedef const char *(*get_input_backend_name_fn)(void);

// Lifecycle.
typedef void (*create_input_backend_fn)(input_backend *backend);
typedef void (*destroy_input_backend_fn)(input_backend *backend);
typedef bool (*setup_input_backend_context_fn)(void *ctx);
typedef void (*close_input_backend_context_fn)(void *ctx);

struct input_backend {
    is_input_backend_available_fn is_available;
    get_input_backend_description_fn get_description;
    get_input_backend_name_fn get_name;

    create_input_backend_fn create;
    destroy_input_backend_fn destroy;

    setup_input_backend_context_fn setup_context;
    close_input_backend_context_fn close_context;

    void *ctx;
};

#endif // INPUT_BACKEND_H
