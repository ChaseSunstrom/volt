#include <lexer/token.h>
#include <pch.h>

#include "util/types/types.h"

volt_allocator_t volt_token_allocator = {NULL, NULL, .free = volt_vtoken_deinit, NULL};

void volt_vtoken_deinit(void* vtoken) {
    volt_token_deinit((volt_token_t*) vtoken);
}

volt_status_code_t volt_token_deinit(volt_token_t* token) {
    volt_allocator_t* allocator = token->allocator ? token->allocator : &volt_default_allocator;
    allocator->free((void*) token->lexeme);
    allocator->free((void*) token);

    return VOLT_SUCCESS;
}

const char* volt_token_type_to_string(volt_token_type_t type) {
    switch (type) {
        /* Single character */
        case VOLT_TOKEN_TYPE_NONE:              return "NONE";
        case VOLT_TOKEN_TYPE_LPAREN:            return "LPAREN";
        case VOLT_TOKEN_TYPE_RPAREN:            return "RPAREN";
        case VOLT_TOKEN_TYPE_LBRACKET:          return "LBRACKET";
        case VOLT_TOKEN_TYPE_RBRACKET:          return "RBRACKET";
        case VOLT_TOKEN_TYPE_LBRACE:            return "LBRACE";
        case VOLT_TOKEN_TYPE_RBRACE:            return "RBRACE";
        case VOLT_TOKEN_TYPE_LANGLE:            return "LANGLE";
        case VOLT_TOKEN_TYPE_RANGLE:            return "RANGLE";
        case VOLT_TOKEN_TYPE_SINGLE_QUOTE:      return "SINGLE_QUOTE";
        case VOLT_TOKEN_TYPE_DOUBLE_QUOTE:      return "DOUBLE_QUOTE";
        case VOLT_TOKEN_TYPE_SLASH:             return "SLASH";
        case VOLT_TOKEN_TYPE_AMPERSAND:         return "AMPERSAND";
        case VOLT_TOKEN_TYPE_AT:                return "AT";
        case VOLT_TOKEN_TYPE_BANG:              return "BANG";
        case VOLT_TOKEN_TYPE_PERCENT:           return "PERCENT";
        case VOLT_TOKEN_TYPE_CARET:             return "CARET";
        case VOLT_TOKEN_TYPE_TILDE:             return "TILDE";
        case VOLT_TOKEN_TYPE_HASH:              return "HASH";
        case VOLT_TOKEN_TYPE_STAR:              return "STAR";
        case VOLT_TOKEN_TYPE_UNDERSCORE:        return "UNDERSCORE";
        case VOLT_TOKEN_TYPE_TACK:              return "TACK";
        case VOLT_TOKEN_TYPE_PLUS:              return "PLUS";
        case VOLT_TOKEN_TYPE_EQUAL:             return "EQUAL";
        case VOLT_TOKEN_TYPE_BAR:               return "BAR";
        case VOLT_TOKEN_TYPE_COLON:             return "COLON";
        case VOLT_TOKEN_TYPE_SEMICOLON:         return "SEMICOLON";
        case VOLT_TOKEN_TYPE_DOT:               return "DOT";
        case VOLT_TOKEN_TYPE_COMMA:             return "COMMA";
        case VOLT_TOKEN_TYPE_QUESTION:          return "QUESTION";

        /* Double character */
        case VOLT_TOKEN_TYPE_EQUAL_EQUAL:         return "EQUAL_EQUAL";
        case VOLT_TOKEN_TYPE_BANG_EQUAL:          return "BANG_EQUAL";
        case VOLT_TOKEN_TYPE_AMPERSAND_EQUAL:     return "AMPERSAND_EQUAL";
        case VOLT_TOKEN_TYPE_TILDE_EQUAL:         return "TILDE_EQUAL";
        case VOLT_TOKEN_TYPE_LANGLE_LANGLE:       return "LANGLE_LANGLE";
        case VOLT_TOKEN_TYPE_RANGLE_RANGLE:       return "RANGLE_RANGLE";
        case VOLT_TOKEN_TYPE_BAR_EQUAL:           return "BAR_EQUAL";
        case VOLT_TOKEN_TYPE_AMPERSAND_AMPERSAND: return "AMPERSAND_AMPERSAND";
        case VOLT_TOKEN_TYPE_BAR_BAR:             return "BAR_BAR";
        case VOLT_TOKEN_TYPE_CARET_EQUAL:         return "CARET_EQUAL";
        case VOLT_TOKEN_TYPE_LANGLE_EQUAL:        return "LANGLE_EQUAL";
        case VOLT_TOKEN_TYPE_RANGLE_EQUAL:        return "RANGLE_EQUAL";
        case VOLT_TOKEN_TYPE_PERCENT_EQUAL:       return "PERCENT_EQUAL";
        case VOLT_TOKEN_TYPE_EQUAL_RANGLE:        return "EQUAL_RANGLE";
        case VOLT_TOKEN_TYPE_PLUS_PLUS:           return "PLUS_PLUS";
        case VOLT_TOKEN_TYPE_TACK_TACK:           return "TACK_TACK";
        case VOLT_TOKEN_TYPE_TACK_RANGLE:         return "TACK_RANGLE";
        case VOLT_TOKEN_TYPE_PLUS_EQUAL:          return "PLUS_EQUAL";
        case VOLT_TOKEN_TYPE_STAR_EQUAL:          return "STAR_EQUAL";
        case VOLT_TOKEN_TYPE_TACK_EQUAL:          return "TACK_EQUAL";
        case VOLT_TOKEN_TYPE_SLASH_EQUAL:         return "SLASH_EQUAL";
        case VOLT_TOKEN_TYPE_SLASH_SLASH:         return "SLASH_SLASH";
        case VOLT_TOKEN_TYPE_SLASH_STAR:          return "SLASH_STAR";
        case VOLT_TOKEN_TYPE_STAR_SLASH:          return "STAR_SLASH";
        case VOLT_TOKEN_TYPE_DOT_DOT:             return "DOT_DOT";
        case VOLT_TOKEN_TYPE_COLON_COLON:         return "COLON_COLON";

        /* Triple character */
        case VOLT_TOKEN_TYPE_DOT_DOT_EQUAL:        return "DOT_DOT_EQUAL";
        case VOLT_TOKEN_TYPE_LANGLE_LANGLE_EQUAL:  return "LANGLE_LANGLE_EQUAL";
        case VOLT_TOKEN_TYPE_RANGLE_RANGLE_EQUAL:  return "RANGLE_RANGLE_EQUAL";

        /* Literals */
        case VOLT_TOKEN_TYPE_NUMBER_LITERAL:      return "NUMBER_LITERAL";
        case VOLT_TOKEN_TYPE_STRING_LITERAL:      return "STRING_LITERAL";
        case VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL:  return "IDENTIFIER_LITERAL";

        /* Keywords */
        case VOLT_TOKEN_TYPE_I8_KW:        return "I8_KW";
        case VOLT_TOKEN_TYPE_I16_KW:       return "I16_KW";
        case VOLT_TOKEN_TYPE_I32_KW:       return "I32_KW";
        case VOLT_TOKEN_TYPE_I64_KW:       return "I64_KW";
        case VOLT_TOKEN_TYPE_I128_KW:      return "I128_KW";
        case VOLT_TOKEN_TYPE_U8_KW:        return "U8_KW";
        case VOLT_TOKEN_TYPE_U16_KW:       return "U16_KW";
        case VOLT_TOKEN_TYPE_U32_KW:       return "U32_KW";
        case VOLT_TOKEN_TYPE_U64_KW:       return "U64_KW";
        case VOLT_TOKEN_TYPE_U128_KW:      return "U128_KW";
        case VOLT_TOKEN_TYPE_F16_KW:       return "F16_KW";
        case VOLT_TOKEN_TYPE_F32_KW:       return "F32_KW";
        case VOLT_TOKEN_TYPE_F64_KW:       return "F64_KW";
        case VOLT_TOKEN_TYPE_F128_KW:      return "F128_KW";
        case VOLT_TOKEN_TYPE_BOOL_KW:      return "BOOL_KW";
        case VOLT_TOKEN_TYPE_ISIZE_KW:     return "ISIZE_KW";
        case VOLT_TOKEN_TYPE_USIZE_KW:     return "USIZE_KW";
        case VOLT_TOKEN_TYPE_TYPE_KW_KW:   return "TYPE_KW_KW";
        case VOLT_TOKEN_TYPE_CSTR_KW:      return "CSTR_KW";
        case VOLT_TOKEN_TYPE_STR_KW:       return "STR_KW";
        case VOLT_TOKEN_TYPE_VAR_KW:       return "VAR_KW";
        case VOLT_TOKEN_TYPE_VAL_KW:       return "VAL_KW";
        case VOLT_TOKEN_TYPE_STATIC_KW:    return "STATIC_KW";
        case VOLT_TOKEN_TYPE_ATTACH_KW:    return "ATTACH_KW";
        case VOLT_TOKEN_TYPE_STRUCT_KW:    return "STRUCT_KW";
        case VOLT_TOKEN_TYPE_ENUM_KW:      return "ENUM_KW";
        case VOLT_TOKEN_TYPE_FN_KW:        return "FN_KW";
        case VOLT_TOKEN_TYPE_ERROR_KW:     return "ERROR_KW";
        case VOLT_TOKEN_TYPE_COMPTIME_KW:  return "COMPTIME_KW";
        case VOLT_TOKEN_TYPE_RETURN_KW:    return "RETURN_KW";
        case VOLT_TOKEN_TYPE_BREAK_KW:     return "BREAK_KW";
        case VOLT_TOKEN_TYPE_CONTINUE_KW:  return "CONTINUE_KW";
        case VOLT_TOKEN_TYPE_INTERN_KW:    return "INTERN_KW";
        case VOLT_TOKEN_TYPE_PUBLIC_KW:    return "PUBLIC_KW";
        case VOLT_TOKEN_TYPE_TRAIT_KW:     return "TRAIT_KW";
        case VOLT_TOKEN_TYPE_ASYNC_KW:     return "ASYNC_KW";
        case VOLT_TOKEN_TYPE_TRUE_KW:      return "TRUE_KW";
        case VOLT_TOKEN_TYPE_FALSE_KW:     return "FALSE_KW";
        case VOLT_TOKEN_TYPE_EXTERN_KW:    return "EXTERN_KW";
        case VOLT_TOKEN_TYPE_EXPORT_KW:    return "EXPORT_KW";
        case VOLT_TOKEN_TYPE_NAMESPACE_KW: return "NAMESPACE_KW";
        case VOLT_TOKEN_TYPE_USE_KW:       return "USE_KW";
        case VOLT_TOKEN_TYPE_THIS_KW:      return "THIS_KW";
        case VOLT_TOKEN_TYPE_MOVE_KW:      return "MOVE_KW";
        case VOLT_TOKEN_TYPE_COPY_KW:      return "COPY_KW";
        case VOLT_TOKEN_TYPE_IF_KW:        return "IF_KW";
        case VOLT_TOKEN_TYPE_ELSE_KW:      return "ELSE_KW";
        case VOLT_TOKEN_TYPE_FOR_KW:       return "FOR_KW";
        case VOLT_TOKEN_TYPE_WHILE_KW:     return "WHILE_KW";
        case VOLT_TOKEN_TYPE_LOOP_KW:      return "LOOP_KW";
        case VOLT_TOKEN_TYPE_TRY_KW:       return "TRY_KW";
        case VOLT_TOKEN_TYPE_CATCH_KW:     return "CATCH_KW";
        case VOLT_TOKEN_TYPE_NULL_KW:      return "NULL_KW";
        case VOLT_TOKEN_TYPE_SUSPEND_KW:   return "SUSPEND_KW";
        case VOLT_TOKEN_TYPE_RESUME_KW:    return "RESUME_KW";

        default: return "UNKNOWN_TOKEN_TYPE";
    }
}

void volt_token_print(volt_token_t* token) {
    volt_fmt_logf(VOLT_FMT_LEVEL_INFO, "Token(Type: '{s}', Lexeme: '{s}', Line: {u64}, Column: {u64})",
                   volt_token_type_to_string(token->type), 
                   token->lexeme, 
                   token->line, token->column);
}