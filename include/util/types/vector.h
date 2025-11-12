#ifndef __VOLT_VECTOR_H__
#define __VOLT_VECTOR_H__

#include <util/types/types.h>

#include "util/memory/allocator.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct volt_vector_t volt_vector_t;
struct volt_vector_t {
    void**            data;
    size_t            size;
    size_t            capacity;
    volt_allocator_t* allocator;
    volt_allocator_t* item_allocator;
};

volt_vector_t      volt_vector_default(void);
volt_status_code_t volt_vector_init(volt_vector_t*);
volt_status_code_t volt_vector_deinit(volt_vector_t*);
volt_status_code_t volt_vector_push_back(volt_vector_t*, void*);
volt_status_code_t volt_vector_pop_back(volt_vector_t*);
volt_status_code_t volt_vector_insert(volt_vector_t*, size_t, void*);
volt_status_code_t volt_vector_remove(volt_vector_t*, size_t);
volt_status_code_t volt_vector_ensure(volt_vector_t*, size_t);
void*              volt_vector_get(volt_vector_t*, size_t);
void*              volt_vector_get_back(volt_vector_t*);

#ifdef __cplusplus
}
#endif

#endif  // __VOLT_VECTOR_H__
