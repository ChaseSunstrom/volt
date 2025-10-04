#include <lexer/token.h>
#include <pch.h>

#include "util/types/types.h"

volt_allocator_t volt_token_allocator = {.free = volt_vtoken_deinit};

void volt_vtoken_deinit(void* vtoken) {
    volt_token_deinit((volt_token_t*) vtoken);
}

int32_t volt_token_deinit(volt_token_t* token) {
    free((void*) token->lexeme);
    return VOLT_SUCCESS;
}
