#ifndef __VOLT_ERROR_H__
#define __VOLT_ERROR_H__

#include <util/types/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef const char* volt_error_t;

typedef struct volt_error_handler_t volt_error_handler_t;
struct volt_error_handler_t {
    volt_error_t* errors;
    int32_t       size;
    int32_t       capacity;
};

int32_t volt_error_handler_push(volt_error_handler_t*, volt_error_t);
int32_t volt_error_handler_pop(volt_error_handler_t*, volt_error_t);
int32_t volt_error_handler_print(volt_error_handler_t*);

#ifdef __cplusplus
}
#endif

#endif  // __VOLT_ERROR_H__
