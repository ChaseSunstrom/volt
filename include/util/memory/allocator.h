#ifndef __VOLT_ALLOCATOR_H__
#define __VOLT_ALLOCATOR_H__

#include <util/types/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* (*volt_malloc_fn)(size_t);
typedef void* (*volt_realloc_fn)(void*, size_t);
typedef void (*volt_free_fn)(void*);

typedef struct volt_allocator_t volt_allocator_t;
struct volt_allocator_t {
    volt_malloc_fn  malloc;
    volt_realloc_fn realloc;
    volt_free_fn    free;
};  // Custom allocators will use this as the first element in a struct (on the
    // stack) (UNSAFE OPS LETS GOOOOOOOOO)

extern volt_allocator_t volt_default_allocator;

#ifdef __cplusplus
}
#endif

#endif  // __VOLT_ALLOCATOR_H__
