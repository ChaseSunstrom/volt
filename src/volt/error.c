#include <pch.h>
#include <volt/error.h>

volt_allocator_t volt_error_allocator = {NULL, NULL, .free = volt_verror_deinit, NULL};

void volt_verror_deinit(void* verror) {
    volt_error_deinit((volt_error_t*) verror);
}

volt_status_code_t volt_error_init(volt_error_handler_t* handler, volt_error_t* error,
                                   volt_error_message_t message, volt_error_type_t error_type,
                                   const char* file, size_t line, size_t column) {
    if (!error)
        return VOLT_FAILURE;

    error->handler = handler;
    error->file    = file;
    error->type    = error_type;
    error->message = strdup(message);
    error->line    = line;
    error->column  = column;

    return VOLT_SUCCESS;
}

volt_status_code_t volt_error_deinit(volt_error_t* error) {
    if (!error)
        return VOLT_FAILURE;

    volt_error_handler_t* handler = error->handler;

    handler->allocator->free((void*) error->message);
    handler->allocator->free(error);

    return VOLT_SUCCESS;
}

volt_status_code_t volt_error_handler_init(volt_error_handler_t* handler,
                                           volt_allocator_t*     allocator) {
    if (!handler)
        return VOLT_FAILURE;

    handler->allocator = allocator ? allocator : &volt_default_allocator;

    volt_vector_t vec  = volt_vector_default();
    vec.allocator      = handler->allocator;
    vec.item_allocator = &volt_error_allocator;

    handler->errors = vec;

    return VOLT_SUCCESS;
}

volt_status_code_t volt_error_handler_deinit(volt_error_handler_t* handler) {
    volt_vector_deinit(&handler->errors);

    return VOLT_SUCCESS;
}

volt_status_code_t volt_error_handler_push_error(volt_error_handler_t* handler,
                                                 volt_error_t*         error) {
    if (!handler)
        return VOLT_FAILURE;

    volt_error_t* error_copy = handler->allocator->malloc(sizeof(volt_error_t));
    if (!error_copy)
        return VOLT_FAILURE;

    memcpy(error_copy, error, sizeof(volt_error_t));

    volt_vector_push_back(&handler->errors, (void*) error_copy);

    return VOLT_SUCCESS;
}

volt_status_code_t volt_error_handler_print(volt_error_handler_t* handler) {
    for (size_t i = 0; i < handler->errors.size; i++) {
        volt_error_t*    error = (volt_error_t*) volt_vector_get(&handler->errors, i);
        volt_fmt_level_t level;
        switch (error->type) {
            case VOLT_ERROR_TYPE_WARNING:
                level = VOLT_FMT_LEVEL_WARN;
                break;
            case VOLT_ERROR_TYPE_ERROR:
            case VOLT_ERROR_TYPE_FATAL:
                level = VOLT_FMT_LEVEL_ERROR;
                break;
        }
        volt_fmt_logf(level, "{s}:{i32}:{i32}: {s}", error->file ? error->file : "<unknown>",
                      error->line, error->column, error->message ? error->message : "<no message>");
    }
    return VOLT_SUCCESS;
}
