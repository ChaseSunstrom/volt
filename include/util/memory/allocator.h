#ifndef __VOLT_ALLOCATOR_H__
#define __VOLT_ALLOCATOR_H__

#include <util/types/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct volt_allocation_pair_t volt_allocation_pair_t;
struct volt_allocation_pair_t {
    void* ptr;
    void* user_data;
};

typedef void* (*volt_malloc_fn)(size_t);
typedef void* (*volt_realloc_fn)(void*, size_t);
typedef void (*volt_free_fn)(void*);
typedef void (*volt_free_pair_fn)(volt_allocation_pair_t*);

typedef struct volt_allocator_t volt_allocator_t;
struct volt_allocator_t {
    volt_malloc_fn  malloc;
    volt_realloc_fn realloc;
    union {
        volt_free_fn      free;
        volt_free_pair_fn free_pair;
    };
    void* user_data;
};  // Custom allocators will use this as the first element in a struct (on the
    // stack) (UNSAFE OPS LETS GOOOOOOOOO)

void* volt_allocator_malloc(size_t);
void* volt_allocator_realloc(void*, size_t);
void  volt_allocator_free(void*);

extern volt_allocator_t volt_default_allocator;

#ifdef __cplusplus
}
#endif

#endif  // __VOLT_ALLOCATOR_H__
