#include <pch.h>
#include <util/memory/allocator.h>

void* volt_allocator_malloc(size_t size) {
    return malloc(size);
}

void* volt_allocator_realloc(void* ptr, size_t size) {
    if (!ptr)
        return malloc(size);
    return realloc(ptr, size);
}

void volt_allocator_free(void* ptr) {
    if (!ptr)
        return;

    free(ptr);
}

volt_allocator_t volt_default_allocator = {.malloc    = volt_allocator_malloc,
                                           .realloc   = volt_allocator_realloc,
                                           .free      = volt_allocator_free,
                                           .user_data = NULL};
