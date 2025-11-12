#include <pch.h>
#include <util/types/vector.h>

#include "util/memory/allocator.h"
#include "util/types/types.h"

volt_vector_t volt_vector_default(void) {
    volt_vector_t vector  = {0};
    vector.size           = 0;
    vector.capacity       = 1;
    vector.allocator      = &volt_default_allocator;
    vector.item_allocator = NULL;
    vector.data           = vector.allocator->malloc(sizeof(void*));
    return vector;
}

volt_status_code_t volt_vector_init(volt_vector_t* v) {
    if (!v || !v->allocator)
        return VOLT_FAILURE;
    if (v->capacity == 0)
        v->capacity = 1;
    v->data = v->allocator->malloc(v->capacity * sizeof(void*));
    return v->data ? VOLT_SUCCESS : VOLT_FAILURE;
}

volt_status_code_t volt_vector_ensure(volt_vector_t* v, size_t extra) {
    if (!v || !v->allocator)
        return VOLT_FAILURE;

    // Initialize lazily if needed
    if (!v->data) {
        v->capacity = v->capacity ? v->capacity : 1;
        v->data     = v->allocator->malloc(v->capacity * sizeof(void*));
        if (!v->data)
            return VOLT_FAILURE;
    }

    // Need at least v->size + extra slots
    if (v->size + extra <= v->capacity)
        return VOLT_SUCCESS;

    size_t new_cap = v->capacity ? v->capacity : 1;
    while (new_cap < v->size + extra)
        new_cap *= 2;

    void** new_data = v->allocator->realloc(v->data, new_cap * sizeof(void*));
    if (!new_data)
        return VOLT_FAILURE;

    v->data     = new_data;
    v->capacity = new_cap;
    return VOLT_SUCCESS;
}

volt_status_code_t volt_vector_insert(volt_vector_t* v, size_t index, void* item) {
    if (!v || index > v->size || volt_vector_ensure(v, v->size - index + 1) != VOLT_SUCCESS)
        return VOLT_FAILURE;

    // shift right by one (safe for index==size; len becomes 0)
    memmove(&v->data[index + 1], &v->data[index], (v->size - index) * sizeof(void*));

    v->data[index] = item;
    v->size += 1;
    return VOLT_SUCCESS;
}

volt_status_code_t volt_vector_push_back(volt_vector_t* v, void* item) {
    return volt_vector_insert(v, v->size, item);
}

volt_status_code_t volt_vector_remove(volt_vector_t* v, size_t index) {
    if (!v || index >= v->size)
        return VOLT_FAILURE;

    void* item = v->data[index];

    // shift left by one: move exactly (size - index - 1) elems
    if (index + 1 < v->size) {
        memmove(&v->data[index], &v->data[index + 1], (v->size - index - 1) * sizeof(void*));
    }

    v->size -= 1;

    if (v->item_allocator && item) {
        v->item_allocator->free(item);
    }
    return VOLT_SUCCESS;
}

volt_status_code_t volt_vector_pop_back(volt_vector_t* v) {
    if (!v || v->size == 0)
        return VOLT_FAILURE;
    return volt_vector_remove(v, v->size - 1);
}

void* volt_vector_get(volt_vector_t* v, size_t index) {
    if (!v || index >= v->size)
        return NULL;
    return v->data[index];
}

void* volt_vector_get_back(volt_vector_t* v) {
    if (!v || v->size == 0)
        return NULL;
    return v->data[v->size - 1];
}

volt_status_code_t volt_vector_deinit(volt_vector_t* v) {
    if (!v || !v->allocator)
        return VOLT_FAILURE;
    if (v->item_allocator) {
        for (size_t i = 0; i < v->size; i++) {
            if (v->data[i])
                v->item_allocator->free(v->data[i]);
        }
    }
    if (v->data)
        v->allocator->free(v->data);
    v->data = NULL;
    v->size = v->capacity = 0;
    return VOLT_SUCCESS;
}
