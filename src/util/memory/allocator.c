#include <pch.h>
#include <util/memory/allocator.h>

volt_allocator_t volt_default_allocator = {malloc, realloc, free};
