#ifndef __VOLT_ERROR_H__
#define __VOLT_ERROR_H__

#include <util/types/types.h>
#include <util/types/vector.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef const char* volt_error_message_t;

typedef enum volt_error_type_t volt_error_type_t;
enum volt_error_type_t {
    VOLT_ERROR_TYPE_WARNING,
    VOLT_ERROR_TYPE_ERROR,
    VOLT_ERROR_TYPE_FATAL,
};

typedef struct volt_error_handler_t volt_error_handler_t;
struct volt_error_handler_t {
    volt_vector_t     errors;
    volt_allocator_t* allocator;
};

typedef struct volt_error_t volt_error_t;
struct volt_error_t {
    volt_error_handler_t* handler;
    volt_error_message_t  message;
    volt_error_type_t     type;
    const char*           file;
    size_t                line;
    size_t                column;
};

volt_status_code_t volt_error_init(volt_error_handler_t* handler, volt_error_t*,
                                   volt_error_message_t, volt_error_type_t, const char*, size_t,
                                   size_t);
void               volt_verror_deinit(void* verror);
volt_status_code_t volt_error_deinit(volt_error_t*);

volt_status_code_t volt_error_handler_init(volt_error_handler_t*, volt_allocator_t*);
volt_status_code_t volt_error_handler_deinit(volt_error_handler_t*);
volt_status_code_t volt_error_handler_push_error(volt_error_handler_t*, volt_error_t*);
volt_status_code_t volt_error_handler_print(volt_error_handler_t*);

extern volt_allocator_t volt_error_allocator;

#ifdef __cplusplus
}
#endif

#endif  // __VOLT_ERROR_H__
