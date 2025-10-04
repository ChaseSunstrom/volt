#include <lexer/lexer.h>
#include <lexer/token.h>
#include <pch.h>

int32_t volt_lexer_init(volt_lexer_t* lexer) {
    lexer->tokens.item_allocator = &volt_token_allocator;
    return VOLT_SUCCESS;
}

int32_t volt_lexer_deinit(volt_lexer_t* lexer) {
    volt_vector_deinit(&lexer->tokens);
    return VOLT_SUCCESS;
}

int32_t volt_lexer_lex(volt_lexer_t* lexer) {
    (void) lexer;
    return VOLT_SUCCESS;
}
