#include <lexer/lexer.h>
#include <lexer/token.h>
#include <pch.h>

typedef struct volt_keyword_map_t volt_keyword_map_t;
struct volt_keyword_map_t {
    const char*       keyword;
    volt_token_type_t token_type;
};

static volt_keyword_map_t keywords[] = {{"i8", VOLT_TOKEN_TYPE_I8_KW},
                                        {"i16", VOLT_TOKEN_TYPE_I16_KW},
                                        {"i32", VOLT_TOKEN_TYPE_I32_KW},
                                        {"i64", VOLT_TOKEN_TYPE_I64_KW},
                                        {"i128", VOLT_TOKEN_TYPE_I128_KW},
                                        {"u8", VOLT_TOKEN_TYPE_U8_KW},
                                        {"u16", VOLT_TOKEN_TYPE_U16_KW},
                                        {"u32", VOLT_TOKEN_TYPE_U32_KW},
                                        {"u64", VOLT_TOKEN_TYPE_U64_KW},
                                        {"u128", VOLT_TOKEN_TYPE_U128_KW},
                                        {"f16", VOLT_TOKEN_TYPE_F16_KW},
                                        {"f32", VOLT_TOKEN_TYPE_F32_KW},
                                        {"f64", VOLT_TOKEN_TYPE_F64_KW},
                                        {"f128", VOLT_TOKEN_TYPE_F128_KW},
                                        {"bool", VOLT_TOKEN_TYPE_BOOL_KW},
                                        {"isize", VOLT_TOKEN_TYPE_ISIZE_KW},
                                        {"usize", VOLT_TOKEN_TYPE_USIZE_KW},
                                        {"type", VOLT_TOKEN_TYPE_TYPE_KW},
                                        {"cstr", VOLT_TOKEN_TYPE_CSTR_KW},
                                        {"str", VOLT_TOKEN_TYPE_STR_KW},
                                        {"var", VOLT_TOKEN_TYPE_VAR_KW},
                                        {"val", VOLT_TOKEN_TYPE_VAL_KW},
                                        {"static", VOLT_TOKEN_TYPE_STATIC_KW},
                                        {"attach", VOLT_TOKEN_TYPE_ATTACH_KW},
                                        {"struct", VOLT_TOKEN_TYPE_STRUCT_KW},
                                        {"enum", VOLT_TOKEN_TYPE_ENUM_KW},
                                        {"fn", VOLT_TOKEN_TYPE_FN_KW},
                                        {"error", VOLT_TOKEN_TYPE_ERROR_KW},
                                        {"comptime", VOLT_TOKEN_TYPE_COMPTIME_KW},
                                        {"return", VOLT_TOKEN_TYPE_RETURN_KW},
                                        {"break", VOLT_TOKEN_TYPE_BREAK_KW},
                                        {"continue", VOLT_TOKEN_TYPE_CONTINUE_KW},
                                        {"internal", VOLT_TOKEN_TYPE_INTERNAL_KW},
                                        {"public", VOLT_TOKEN_TYPE_PUBLIC_KW},
                                        {"trait", VOLT_TOKEN_TYPE_TRAIT_KW},
                                        {"async", VOLT_TOKEN_TYPE_ASYNC_KW},
                                        {"true", VOLT_TOKEN_TYPE_TRUE_KW},
                                        {"false", VOLT_TOKEN_TYPE_FALSE_KW},
                                        {"extern", VOLT_TOKEN_TYPE_EXTERN_KW},
                                        {"export", VOLT_TOKEN_TYPE_EXPORT_KW},
                                        {"namespace", VOLT_TOKEN_TYPE_NAMESPACE_KW},
                                        {"use", VOLT_TOKEN_TYPE_USE_KW},
                                        {"this", VOLT_TOKEN_TYPE_THIS_KW},
                                        {"move", VOLT_TOKEN_TYPE_MOVE_KW},
                                        {"copy", VOLT_TOKEN_TYPE_COPY_KW},
                                        {"if", VOLT_TOKEN_TYPE_IF_KW},
                                        {"else", VOLT_TOKEN_TYPE_ELSE_KW},
                                        {"for", VOLT_TOKEN_TYPE_FOR_KW},
                                        {"while", VOLT_TOKEN_TYPE_WHILE_KW},
                                        {"loop", VOLT_TOKEN_TYPE_LOOP_KW},
                                        {"try", VOLT_TOKEN_TYPE_TRY_KW},
                                        {"catch", VOLT_TOKEN_TYPE_CATCH_KW},
                                        {"in", VOLT_TOKEN_TYPE_IN_KW},
                                        {"null", VOLT_TOKEN_TYPE_NULL_KW},
                                        {"suspend", VOLT_TOKEN_TYPE_SUSPEND_KW},
                                        {"resume", VOLT_TOKEN_TYPE_RESUME_KW},
                                        {"defer", VOLT_TOKEN_TYPE_DEFER_KW},
                                        {"as", VOLT_TOKEN_TYPE_AS_KW},
                                        {NULL, 0}};

volt_status_code_t volt_lexer_init(volt_lexer_t* lexer, volt_allocator_t* allocator) {
    if (!lexer) {
        return VOLT_FAILURE;
    }
    lexer->tokens                = volt_vector_default();
    lexer->tokens.item_allocator = &volt_token_allocator;
    lexer->tokens.allocator      = allocator ? allocator : &volt_default_allocator;
    lexer->allocator             = allocator ? allocator : &volt_default_allocator;
    lexer->input_stream_length   = strlen(lexer->input_stream);
    lexer->current_position      = 0;
    lexer->current_line          = 1;
    lexer->current_column        = 1;
    return VOLT_SUCCESS;
}

volt_status_code_t volt_lexer_deinit(volt_lexer_t* lexer) {
    if (!lexer) {
        return VOLT_FAILURE;
    }

    // Debug token printing
    /*
    for (size_t i = 0; i < lexer->tokens.size; i++) {
        volt_token_t* token = (volt_token_t*) volt_vector_get(&lexer->tokens, i);
        volt_token_print(token);
    }
    */

    volt_vector_deinit(&lexer->tokens);
    lexer->allocator->free((void*) lexer->input_stream);
    return VOLT_SUCCESS;
}

bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

bool is_alnum(char c) {
    return is_alpha(c) || is_digit(c);
}

bool is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool match_char(volt_lexer_t* lexer, char expected) {
    size_t next = lexer->current_position + 1;
    if (next >= lexer->input_stream_length)
        return false;
    if (lexer->input_stream[next] != expected)
        return false;

    // consume the next char
    lexer->current_position = next;
    lexer->current_column++;
    return true;
}

volt_token_t* create_token(volt_lexer_t* lexer, volt_token_type_t type, const char* lexeme,
                           size_t start_line, size_t start_column) {
    volt_token_t* token = (volt_token_t*) lexer->allocator->malloc(sizeof(volt_token_t));
    if (!token)
        return NULL;

    token->allocator = lexer->allocator;
    token->type      = type;
    token->lexeme    = strdup(lexeme);
    token->line      = start_line;
    token->column    = start_column;
    return token;
}

volt_token_t* token_identifier_or_kw(volt_lexer_t* lexer, const char* lexeme, size_t start_line,
                                     size_t start_col) {
    for (size_t i = 0; keywords[i].keyword != NULL; i++) {
        if (strcmp(lexeme, keywords[i].keyword) == 0) {
            return create_token(lexer, keywords[i].token_type, lexeme, start_line, start_col);
        }
    }
    return create_token(lexer, VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL, lexeme, start_line, start_col);
}

volt_token_t* token_identifier(volt_lexer_t* lexer) {
    size_t start_line = lexer->current_line;
    size_t start_col  = lexer->current_column;
    size_t start      = lexer->current_position;

    while (lexer->current_position < lexer->input_stream_length &&
           is_alnum(lexer->input_stream[lexer->current_position])) {
        lexer->current_position++;
        lexer->current_column++;
    }

    size_t len    = lexer->current_position - start;
    char*  lexeme = (char*) lexer->allocator->malloc(len + 1);
    strncpy(lexeme, &lexer->input_stream[start], len);
    lexeme[len] = '\0';

    volt_token_t* token = token_identifier_or_kw(lexer, lexeme, start_line, start_col);
    lexer->allocator->free(lexeme);
    return token;
}

volt_token_t* token_number(volt_lexer_t* lexer) {
    size_t start_line = lexer->current_line;
    size_t start_col  = lexer->current_column;
    size_t start      = lexer->current_position;

    while (lexer->current_position < lexer->input_stream_length &&
           is_digit(lexer->input_stream[lexer->current_position])) {
        lexer->current_position++;
        lexer->current_column++;
    }

    // Check for decimal point, but NOT if it's followed by another dot (range operator)
    if (lexer->current_position < lexer->input_stream_length &&
        lexer->input_stream[lexer->current_position] == '.') {
        // Look ahead to see if it's '..' (range operator)
        char next_char = (lexer->current_position + 1 < lexer->input_stream_length)
                             ? lexer->input_stream[lexer->current_position + 1]
                             : '\0';

        // Only consume '.' if it's NOT followed by another '.' (not a range operator)
        if (next_char != '.') {
            // consume '.'
            lexer->current_position++;
            lexer->current_column++;
            while (lexer->current_position < lexer->input_stream_length &&
                   is_digit(lexer->input_stream[lexer->current_position])) {
                lexer->current_position++;
                lexer->current_column++;
            }
        }
        // If next_char is '.', we don't consume anything - let it be tokenized as '..'
    }

    size_t len    = lexer->current_position - start;
    char*  lexeme = (char*) lexer->allocator->malloc(len + 1);
    strncpy(lexeme, &lexer->input_stream[start], len);
    lexeme[len] = '\0';

    volt_token_t* token =
        create_token(lexer, VOLT_TOKEN_TYPE_NUMBER_LITERAL, lexeme, start_line, start_col);
    lexer->allocator->free(lexeme);
    return token;
}

volt_token_t* token_string(volt_lexer_t* lexer, char quote_type) {
    size_t start_line = lexer->current_line;
    size_t start_col =
        lexer->current_column - 1;           // caller consumes opening quote and advanced column
    size_t start = lexer->current_position;  // points at first char of string content

    while (lexer->current_position < lexer->input_stream_length) {
        char c = lexer->input_stream[lexer->current_position];
        if (c == quote_type)
            break;
        if (c == '\n') {
            lexer->current_line++;
            lexer->current_column = 1;
            lexer->current_position++;
            continue;
        }
        // For now, don't special-case escapes; you can add handling here.
        lexer->current_position++;
        lexer->current_column++;
    }

    size_t len    = lexer->current_position - start;
    char*  lexeme = (char*) lexer->allocator->malloc(len + 1);
    strncpy(lexeme, &lexer->input_stream[start], len);
    lexeme[len] = '\0';

    // consume closing quote if present
    if (lexer->current_position < lexer->input_stream_length &&
        lexer->input_stream[lexer->current_position] == quote_type) {
        lexer->current_position++;
        lexer->current_column++;
    }

    volt_token_t* token =
        create_token(lexer, VOLT_TOKEN_TYPE_STRING_LITERAL, lexeme, start_line, start_col);
    lexer->allocator->free(lexeme);
    return token;
}

volt_status_code_t volt_lexer_lex(volt_lexer_t* lexer) {
    if (!lexer) {
        return VOLT_FAILURE;
    }

    while (lexer->current_position < lexer->input_stream_length) {
        char current_char = lexer->input_stream[lexer->current_position];

        /* helper macros to record start position */
        size_t start_line = lexer->current_line;
        size_t start_col  = lexer->current_column;

        /* convenience peek */
        char next_char = (lexer->current_position + 1 < lexer->input_stream_length)
                             ? lexer->input_stream[lexer->current_position + 1]
                             : '\0';

        switch (current_char) {
            case ' ':
            case '\t':
            case '\r':
                lexer->current_position++;
                lexer->current_column++;
                break;

            case '\n':
                lexer->current_position++;
                lexer->current_line++;
                lexer->current_column = 1;
                break;

            case '+': {
                if (next_char == '+') {
                    volt_token_t* token =
                        create_token(lexer, VOLT_TOKEN_TYPE_PLUS_PLUS, "++", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position += 2;
                    lexer->current_column += 2;
                } else if (next_char == '=') {
                    volt_token_t* token = create_token(lexer, VOLT_TOKEN_TYPE_PLUS_EQUAL,
                                                       "+=", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position += 2;
                    lexer->current_column += 2;
                } else {
                    volt_token_t* token =
                        create_token(lexer, VOLT_TOKEN_TYPE_PLUS, "+", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position++;
                    lexer->current_column++;
                }
                break;
            }

            case '-': {
                if (next_char == '-') {
                    volt_token_t* token =
                        create_token(lexer, VOLT_TOKEN_TYPE_TACK_TACK, "--", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position += 2;
                    lexer->current_column += 2;
                } else if (next_char == '=') {
                    volt_token_t* token = create_token(lexer, VOLT_TOKEN_TYPE_TACK_EQUAL,
                                                       "-=", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position += 2;
                    lexer->current_column += 2;
                } else if (next_char == '>') {
                    volt_token_t* token = create_token(lexer, VOLT_TOKEN_TYPE_TACK_RANGLE, "->",
                                                       start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position += 2;
                    lexer->current_column += 2;
                } else {
                    volt_token_t* token =
                        create_token(lexer, VOLT_TOKEN_TYPE_TACK, "-", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position++;
                    lexer->current_column++;
                }
                break;
            }

            case '*': {
                if (next_char == '=') {
                    volt_token_t* token = create_token(lexer, VOLT_TOKEN_TYPE_STAR_EQUAL,
                                                       "*=", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position += 2;
                    lexer->current_column += 2;
                } else {
                    volt_token_t* token =
                        create_token(lexer, VOLT_TOKEN_TYPE_STAR, "*", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position++;
                    lexer->current_column++;
                }
                break;
            }

            case '/': {
                if (next_char == '=') {
                    volt_token_t* token = create_token(lexer, VOLT_TOKEN_TYPE_SLASH_EQUAL,
                                                       "/=", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position += 2;
                    lexer->current_column += 2;
                } else if (next_char == '/') {
                    /* line comment: consume '//' and rest of line */
                    lexer->current_position += 2;
                    lexer->current_column += 2;
                    while (lexer->current_position < lexer->input_stream_length &&
                           lexer->input_stream[lexer->current_position] != '\n') {
                        lexer->current_position++;
                        lexer->current_column++;
                    }
                } else if (next_char == '*') {
                    /* block comment: */
                    lexer->current_position += 2;
                    lexer->current_column += 2;
                    while (lexer->current_position < lexer->input_stream_length) {
                        if (lexer->input_stream[lexer->current_position] == '*' &&
                            lexer->current_position + 1 < lexer->input_stream_length &&
                            lexer->input_stream[lexer->current_position + 1] == '/') {
                            lexer->current_position += 2;
                            lexer->current_column += 2;
                            break;
                        }
                        if (lexer->input_stream[lexer->current_position] == '\n') {
                            lexer->current_position++;
                            lexer->current_line++;
                            lexer->current_column = 1;
                        } else {
                            lexer->current_position++;
                            lexer->current_column++;
                        }
                    }
                } else {
                    volt_token_t* token =
                        create_token(lexer, VOLT_TOKEN_TYPE_SLASH, "/", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position++;
                    lexer->current_column++;
                }
                break;
            }

            case '=': {
                if (next_char == '=') {
                    volt_token_t* token = create_token(lexer, VOLT_TOKEN_TYPE_EQUAL_EQUAL,
                                                       "==", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position += 2;
                    lexer->current_column += 2;
                } else if (next_char == '>') {
                    volt_token_t* token = create_token(lexer, VOLT_TOKEN_TYPE_EQUAL_RANGLE, "=>",
                                                       start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position += 2;
                    lexer->current_column += 2;
                } else {
                    volt_token_t* token =
                        create_token(lexer, VOLT_TOKEN_TYPE_EQUAL, "=", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position++;
                    lexer->current_column++;
                }
                break;
            }

            case '%': {
                if (next_char == '=') {
                    volt_token_t* token = create_token(lexer, VOLT_TOKEN_TYPE_PERCENT_EQUAL,
                                                       "%=", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position += 2;
                    lexer->current_column += 2;
                } else {
                    volt_token_t* token =
                        create_token(lexer, VOLT_TOKEN_TYPE_PERCENT, "%", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position++;
                    lexer->current_column++;
                }
                break;
            }

            case '.': {
                if (next_char == '.') {
                    /* .. or ..= */
                    if (lexer->current_position + 2 < lexer->input_stream_length &&
                        lexer->input_stream[lexer->current_position + 2] == '=') {
                        volt_token_t* token = create_token(lexer, VOLT_TOKEN_TYPE_DOT_DOT_EQUAL,
                                                           "..=", start_line, start_col);
                        volt_vector_push_back(&lexer->tokens, token);
                        lexer->current_position += 3;
                        lexer->current_column += 3;
                    } else {
                        volt_token_t* token = create_token(lexer, VOLT_TOKEN_TYPE_DOT_DOT, "..",
                                                           start_line, start_col);
                        volt_vector_push_back(&lexer->tokens, token);
                        lexer->current_position += 2;
                        lexer->current_column += 2;
                    }
                } else {
                    volt_token_t* token =
                        create_token(lexer, VOLT_TOKEN_TYPE_DOT, ".", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position++;
                    lexer->current_column++;
                }
                break;
            }

            case ':': {
                if (next_char == ':') {
                    volt_token_t* token = create_token(lexer, VOLT_TOKEN_TYPE_COLON_COLON,
                                                       "::", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position += 2;
                    lexer->current_column += 2;
                } else {
                    volt_token_t* token =
                        create_token(lexer, VOLT_TOKEN_TYPE_COLON, ":", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position++;
                    lexer->current_column++;
                }
                break;
            }

            case '(': {
                volt_token_t* token =
                    create_token(lexer, VOLT_TOKEN_TYPE_LPAREN, "(", start_line, start_col);
                volt_vector_push_back(&lexer->tokens, token);
                lexer->current_position++;
                lexer->current_column++;
                break;
            }

            case ')': {
                volt_token_t* token =
                    create_token(lexer, VOLT_TOKEN_TYPE_RPAREN, ")", start_line, start_col);
                volt_vector_push_back(&lexer->tokens, token);
                lexer->current_position++;
                lexer->current_column++;
                break;
            }

            case '{': {
                volt_token_t* token =
                    create_token(lexer, VOLT_TOKEN_TYPE_LBRACE, "{", start_line, start_col);
                volt_vector_push_back(&lexer->tokens, token);
                lexer->current_position++;
                lexer->current_column++;
                break;
            }

            case '}': {
                volt_token_t* token =
                    create_token(lexer, VOLT_TOKEN_TYPE_RBRACE, "}", start_line, start_col);
                volt_vector_push_back(&lexer->tokens, token);
                lexer->current_position++;
                lexer->current_column++;
                break;
            }

            case '[': {
                volt_token_t* token =
                    create_token(lexer, VOLT_TOKEN_TYPE_LBRACKET, "[", start_line, start_col);
                volt_vector_push_back(&lexer->tokens, token);
                lexer->current_position++;
                lexer->current_column++;
                break;
            }

            case ']': {
                volt_token_t* token =
                    create_token(lexer, VOLT_TOKEN_TYPE_RBRACKET, "]", start_line, start_col);
                volt_vector_push_back(&lexer->tokens, token);
                lexer->current_position++;
                lexer->current_column++;
                break;
            }

            case '<': {
                if (next_char == '<') {
                    if (lexer->current_position + 2 < lexer->input_stream_length &&
                        lexer->input_stream[lexer->current_position + 2] == '=') {
                        volt_token_t* token =
                            create_token(lexer, VOLT_TOKEN_TYPE_LANGLE_LANGLE_EQUAL,
                                         "<<=", start_line, start_col);
                        volt_vector_push_back(&lexer->tokens, token);
                        lexer->current_position += 3;
                        lexer->current_column += 3;
                    } else {
                        volt_token_t* token = create_token(lexer, VOLT_TOKEN_TYPE_LANGLE_LANGLE,
                                                           "<<", start_line, start_col);
                        volt_vector_push_back(&lexer->tokens, token);
                        lexer->current_position += 2;
                        lexer->current_column += 2;
                    }
                } else if (next_char == '=') {
                    volt_token_t* token = create_token(lexer, VOLT_TOKEN_TYPE_LANGLE_EQUAL,
                                                       "<=", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position += 2;
                    lexer->current_column += 2;
                } else {
                    volt_token_t* token =
                        create_token(lexer, VOLT_TOKEN_TYPE_LANGLE, "<", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position++;
                    lexer->current_column++;
                }
                break;
            }

            case '>': {
                if (next_char == '>') {
                    if (lexer->current_position + 2 < lexer->input_stream_length &&
                        lexer->input_stream[lexer->current_position + 2] == '=') {
                        volt_token_t* token =
                            create_token(lexer, VOLT_TOKEN_TYPE_RANGLE_RANGLE_EQUAL,
                                         ">>=", start_line, start_col);
                        volt_vector_push_back(&lexer->tokens, token);
                        lexer->current_position += 3;
                        lexer->current_column += 3;
                    } else {
                        volt_token_t* token = create_token(lexer, VOLT_TOKEN_TYPE_RANGLE_RANGLE,
                                                           ">>", start_line, start_col);
                        volt_vector_push_back(&lexer->tokens, token);
                        lexer->current_position += 2;
                        lexer->current_column += 2;
                    }
                } else if (next_char == '=') {
                    volt_token_t* token = create_token(lexer, VOLT_TOKEN_TYPE_RANGLE_EQUAL,
                                                       ">=", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position += 2;
                    lexer->current_column += 2;
                } else {
                    volt_token_t* token =
                        create_token(lexer, VOLT_TOKEN_TYPE_RANGLE, ">", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position++;
                    lexer->current_column++;
                }
                break;
            }

            case '@': {
                volt_token_t* token =
                    create_token(lexer, VOLT_TOKEN_TYPE_AT, "@", start_line, start_col);
                volt_vector_push_back(&lexer->tokens, token);
                lexer->current_position++;
                lexer->current_column++;
                break;
            }

            case '!': {
                if (next_char == '=') {
                    volt_token_t* token = create_token(lexer, VOLT_TOKEN_TYPE_BANG_EQUAL,
                                                       "!=", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position += 2;
                    lexer->current_column += 2;
                } else {
                    volt_token_t* token =
                        create_token(lexer, VOLT_TOKEN_TYPE_BANG, "!", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position++;
                    lexer->current_column++;
                }
                break;
            }

            case '#': {
                volt_token_t* token =
                    create_token(lexer, VOLT_TOKEN_TYPE_HASH, "#", start_line, start_col);
                volt_vector_push_back(&lexer->tokens, token);
                lexer->current_position++;
                lexer->current_column++;
                break;
            }

            case '^': {
                if (next_char == '=') {
                    volt_token_t* token = create_token(lexer, VOLT_TOKEN_TYPE_CARET_EQUAL,
                                                       "^=", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position += 2;
                    lexer->current_column += 2;
                } else {
                    volt_token_t* token =
                        create_token(lexer, VOLT_TOKEN_TYPE_CARET, "^", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position++;
                    lexer->current_column++;
                }
                break;
            }

            case '~': {
                if (next_char == '=') {
                    volt_token_t* token = create_token(lexer, VOLT_TOKEN_TYPE_TILDE_EQUAL,
                                                       "~=", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position += 2;
                    lexer->current_column += 2;
                } else {
                    volt_token_t* token =
                        create_token(lexer, VOLT_TOKEN_TYPE_TILDE, "~", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position++;
                    lexer->current_column++;
                }
                break;
            }

            case '&': {
                if (next_char == '&') {
                    volt_token_t* token = create_token(lexer, VOLT_TOKEN_TYPE_AMPERSAND_AMPERSAND,
                                                       "&&", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position += 2;
                    lexer->current_column += 2;
                } else if (next_char == '=') {
                    volt_token_t* token = create_token(lexer, VOLT_TOKEN_TYPE_AMPERSAND_EQUAL,
                                                       "&=", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position += 2;
                    lexer->current_column += 2;
                } else {
                    volt_token_t* token =
                        create_token(lexer, VOLT_TOKEN_TYPE_AMPERSAND, "&", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position++;
                    lexer->current_column++;
                }
                break;
            }

            case '|': {
                if (next_char == '|') {
                    volt_token_t* token =
                        create_token(lexer, VOLT_TOKEN_TYPE_BAR_BAR, "||", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position += 2;
                    lexer->current_column += 2;
                } else if (next_char == '=') {
                    volt_token_t* token =
                        create_token(lexer, VOLT_TOKEN_TYPE_BAR_EQUAL, "|=", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position += 2;
                    lexer->current_column += 2;
                } else {
                    volt_token_t* token =
                        create_token(lexer, VOLT_TOKEN_TYPE_BAR, "|", start_line, start_col);
                    volt_vector_push_back(&lexer->tokens, token);
                    lexer->current_position++;
                    lexer->current_column++;
                }
                break;
            }

            case ',': {
                volt_token_t* token =
                    create_token(lexer, VOLT_TOKEN_TYPE_COMMA, ",", start_line, start_col);
                volt_vector_push_back(&lexer->tokens, token);
                lexer->current_position++;
                lexer->current_column++;
                break;
            }

            case ';': {
                volt_token_t* token =
                    create_token(lexer, VOLT_TOKEN_TYPE_SEMICOLON, ";", start_line, start_col);
                volt_vector_push_back(&lexer->tokens, token);
                lexer->current_position++;
                lexer->current_column++;
                break;
            }

            case '?': {
                volt_token_t* token =
                    create_token(lexer, VOLT_TOKEN_TYPE_QUESTION, "?", start_line, start_col);
                volt_vector_push_back(&lexer->tokens, token);
                lexer->current_position++;
                lexer->current_column++;
                break;
            }

            default: {
                if (is_alpha(current_char)) {
                    /* token_identifier consumes characters and advances
                     * lexer->current_position/column */
                    volt_token_t* token = token_identifier(lexer);
                    volt_vector_push_back(&lexer->tokens, token);
                } else if (is_digit(current_char)) {
                    volt_token_t* token = token_number(lexer);
                    volt_vector_push_back(&lexer->tokens, token);
                } else if (current_char == '"' || current_char == '\'') {
                    /* consume opening quote then let token_string handle the content+closing quote
                     */
                    char quote_type = current_char;
                    lexer->current_position++;
                    lexer->current_column++;
                    volt_token_t* token = token_string(lexer, quote_type);
                    volt_vector_push_back(&lexer->tokens, token);
                } else {
                    volt_error_t error = {0};
                    volt_error_init(lexer->error_handler, &error,
                                    "Unexpected character in input stream", VOLT_ERROR_TYPE_ERROR,
                                    lexer->input_stream_name, lexer->current_line,
                                    lexer->current_column);
                    volt_error_handler_push_error(lexer->error_handler, &error);
                    /* prevent infinite loop by advancing one */
                    lexer->current_position++;
                    lexer->current_column++;
                }
                break;
            }
        } /* switch */
    } /* while */

    return VOLT_SUCCESS;
}
