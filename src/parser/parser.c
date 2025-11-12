#include <parser/expression.h>
#include <parser/parser.h>
#include <pch.h>

// HELPER FUNCTIONS
static volt_token_t* volt_parser_peek(volt_parser_t* parser, size_t offset) {
    size_t index = parser->current + offset;
    if (index >= parser->tokens->size) {
        return NULL;
    }
    return (volt_token_t*) volt_vector_get(parser->tokens, index);
}

static volt_token_t* volt_parser_current_token(volt_parser_t* parser) {
    return volt_parser_peek(parser, 0);
}

static bool volt_parser_is_at_end(volt_parser_t* parser) {
    return parser->current >= parser->tokens->size;
}

static void volt_parser_advance(volt_parser_t* parser) {
    if (!volt_parser_is_at_end(parser)) {
        parser->current++;
    }
}

static void volt_parser_error(volt_parser_t* parser, const char* message) {
    // Only track the error that got furthest
    if (parser->current > parser->furthest_error_pos) {
        parser->furthest_error_pos = parser->current;
        snprintf(parser->furthest_error_msg, sizeof(parser->furthest_error_msg), "%s", message);
        parser->error_reported = false;
    }
}

static void volt_parser_report_error(volt_parser_t* parser) {
    if (parser->error_reported)
        return;

    volt_error_t  error = {0};
    volt_token_t* token =
        (volt_token_t*) volt_vector_get(parser->tokens, parser->furthest_error_pos);

    if (!token && parser->furthest_error_pos > 0) {
        token = (volt_token_t*) volt_vector_get(parser->tokens, parser->furthest_error_pos - 1);
    }

    if (token) {
        volt_error_init(parser->error_handler, &error, parser->furthest_error_msg,
                        VOLT_ERROR_TYPE_ERROR, parser->input_stream_name, token->line,
                        token->column);
        volt_error_handler_push_error(parser->error_handler, &error);
    }

    parser->error_reported = true;
}

// AST NODE FUNCTIONS
volt_ast_node_t* volt_ast_node_create(volt_parser_t* parser, volt_ast_node_type_t type,
                                      const char* expression_name) {
    volt_ast_node_t* node = (volt_ast_node_t*) parser->allocator->malloc(sizeof(volt_ast_node_t));

    if (!node)
        return NULL;

    node->type            = type;
    node->expression_name = expression_name;
    node->token           = NULL;

    volt_vector_t vector = {0};
    vector.allocator     = parser->allocator;
    volt_vector_init(&vector);

    node->children = vector;
    node->data     = NULL;

    return node;
}

void volt_ast_node_add_child(volt_ast_node_t* parent, volt_ast_node_t* child) {
    if (parent && child) {
        volt_vector_push_back(&parent->children, child);
    }
}

void volt_ast_node_free(volt_parser_t* parser, volt_ast_node_t* node) {
    if (!node)
        return;

    for (size_t i = 0; i < node->children.size; i++) {
        volt_ast_node_t* child = (volt_ast_node_t*) volt_vector_get(&node->children, i);
        volt_ast_node_free(parser, child);
    }

    volt_vector_deinit(&node->children);

    parser->allocator->free(node);
}

// PARSING FUNCTIONS
// Forward declaration
static volt_ast_node_t* volt_parser_parse_expression(volt_parser_t*     parser,
                                                     volt_expression_t* expr);

// Try to match a specific token type
static volt_ast_node_t* volt_parser_match_token(volt_parser_t* parser, volt_token_type_t expected) {
    volt_token_t* token = volt_parser_current_token(parser);

    if (!token) {
        volt_parser_error(parser, "Unexpected end of input");
        return NULL;
    }

    if (token->type != expected) {
        return NULL;
    }

    // Create AST node for token
    volt_ast_node_t* node = volt_ast_node_create(parser, VOLT_AST_NODE_TOKEN, "token");
    node->token           = token;

    volt_parser_advance(parser);
    return node;
}

// Try to parse a subexpression by name
static volt_ast_node_t* volt_parser_parse_subexpression(volt_parser_t* parser,
                                                        const char*    expr_name) {
    volt_expression_t* expr = volt_expression_registry_get(parser->registry, expr_name);

    if (!expr) {
        volt_parser_error(parser, "Unknown expression");
        return NULL;
    }

    return volt_parser_parse_expression(parser, expr);
}

// Try to parse a single alternative
static volt_ast_node_t* volt_parser_try_alternative(volt_parser_t* parser, volt_expression_t* expr,
                                                    size_t alt_index) {
    size_t                saved_position = parser->current;
    volt_subexpression_t* alt            = expr->alternatives[alt_index];
    size_t                alt_len        = expr->alternative_lengths[alt_index];

    // Create parent node for this alternative
    volt_ast_node_t* parent =
        volt_ast_node_create(parser, VOLT_AST_NODE_EXPRESSION, expr->expression_name);

    // Try to match each element in the alternative
    for (size_t i = 0; i < alt_len; i++) {
        volt_ast_node_t* child = NULL;

        if (alt[i].is_subexpression) {
            // Parse subexpression
            child = volt_parser_parse_subexpression(parser, alt[i].subexpression);

            if (!child) {
                if (alt[i].is_optional) {
                    // Optional element not present - that's OK
                    continue;
                } else {
                    // Required element failed - backtrack
                    volt_ast_node_free(parser, parent);
                    parser->current = saved_position;
                    return NULL;
                }
            }
        } else {
            // Match token
            child = volt_parser_match_token(parser, alt[i].token_type);

            if (!child) {
                if (alt[i].is_optional) {
                    // Optional token not present - that's OK
                    continue;
                } else {
                    // Required token failed - backtrack
                    volt_ast_node_free(parser, parent);
                    parser->current = saved_position;
                    return NULL;
                }
            }
        }

        // Add successfully parsed child
        if (child) {
            volt_ast_node_add_child(parent, child);
        }
    }

    // Success!
    return parent;
}

// Parse an expression by trying all alternatives
static volt_ast_node_t* volt_parser_parse_expression(volt_parser_t*     parser,
                                                     volt_expression_t* expr) {
    if (!expr)
        return NULL;

    // Try each alternative in order
    for (size_t i = 0; i < expr->num_alternatives; i++) {
        volt_ast_node_t* result = volt_parser_try_alternative(parser, expr, i);

        if (result) {
            return result;
        }
        // If this alternative failed, try the next one
    }

    // None of the alternatives matched - track error
    char error_msg[256];
    snprintf(error_msg, sizeof(error_msg), "Failed to parse '%s'", expr->expression_name);
    volt_parser_error(parser, error_msg);
    return NULL;
}

volt_status_code_t volt_parser_init(volt_parser_t* parser, volt_allocator_t* allocator,
                                    volt_vector_t* tokens, volt_error_handler_t* error_handler,
                                    const char* input_stream_name) {
    static volt_expression_registry_t expression_registry  = {0};
    static bool                       registry_initialized = false;

    if (!registry_initialized) {
        volt_expression_registry_init(&expression_registry, allocator);
        volt_define_expressions(&expression_registry);
        registry_initialized = true;
    }

    parser->allocator         = allocator ? allocator : &volt_default_allocator;
    parser->tokens            = tokens;
    parser->current           = 0;
    parser->registry          = &expression_registry;
    parser->root              = NULL;
    parser->error_handler     = error_handler;
    parser->input_stream_name = input_stream_name;

    // Initialize error tracking
    parser->furthest_error_pos    = 0;
    parser->furthest_error_msg[0] = '\0';
    parser->error_reported        = false;

    return VOLT_SUCCESS;
}

volt_status_code_t volt_parser_parse(volt_parser_t* parser) {
    if (!parser || !parser->tokens || !parser->registry) {
        return VOLT_FAILURE;
    }

    // Start parsing from the "unit" expression (top-level)
    volt_expression_t* unit_expr = volt_expression_registry_get(parser->registry, "unit");

    if (!unit_expr) {
        volt_parser_error(parser, "No 'unit' expression defined in grammar");
        volt_parser_report_error(parser);  // Report immediately
        return VOLT_FAILURE;
    }

    // Parse the entire input
    parser->root = volt_parser_parse_expression(parser, unit_expr);

    if (!parser->root) {
        volt_parser_report_error(parser);  // Report the furthest error
        return VOLT_FAILURE;
    }

    // Check if we consumed all tokens
    if (!volt_parser_is_at_end(parser)) {
        volt_parser_error(parser, "Unexpected tokens after end of input");
        volt_parser_report_error(parser);
        return VOLT_FAILURE;
    }

    return VOLT_SUCCESS;
}

void volt_ast_print_tree(volt_ast_node_t* node, int32_t indent) {
    if (!node)
        return;

    // Print indentation
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }

    // Print node info
    if (node->type == VOLT_AST_NODE_TOKEN) {
        printf("TOKEN: %s", volt_token_type_to_string(node->token->type));

        // Print token value for literals and identifiers
        if (node->token->type == VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL ||
            node->token->type == VOLT_TOKEN_TYPE_NUMBER_LITERAL ||
            node->token->type == VOLT_TOKEN_TYPE_STRING_LITERAL) {
            printf(" = \"%.*s\"", (int32_t) strlen(node->token->lexeme), node->token->lexeme);
        }
        printf("\n");
    } else if (node->type == VOLT_AST_NODE_EMPTY) {
        printf("EMPTY\n");
    } else {
        printf("EXPR: %s (%zu children)\n",
               node->expression_name ? node->expression_name : "unnamed", node->children.size);
    }

    // Print children recursively
    for (size_t i = 0; i < node->children.size; i++) {
        volt_ast_node_t* child = (volt_ast_node_t*) volt_vector_get(&node->children, i);
        volt_ast_print_tree(child, indent + 1);
    }
}

volt_status_code_t volt_parser_deinit(volt_parser_t* parser) {
    if (!parser)
        return VOLT_FAILURE;

    // Free AST
    if (parser->root) {
        volt_ast_node_free(parser, parser->root);
        parser->root = NULL;
    }

    return VOLT_SUCCESS;
}