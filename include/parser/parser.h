// parser.h
#ifndef __VOLT_PARSER_H__
#define __VOLT_PARSER_H__

#include <lexer/token.h>
#include <parser/expression.h>
#include <pch.h>
#include <util/memory/allocator.h>
#include <util/types/vector.h>
#include <volt/error.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct volt_parser_t      volt_parser_t;
typedef struct volt_ast_node_t    volt_ast_node_t;
typedef enum volt_ast_node_type_t volt_ast_node_type_t;

// AST Node types
enum volt_ast_node_type_t {
    VOLT_AST_NODE_TOKEN,       // Leaf node (terminal from lexer)
    VOLT_AST_NODE_EXPRESSION,  // Internal node (non-terminal from grammar)
    VOLT_AST_NODE_EMPTY,       // Epsilon production (optional)
};

// AST Node structure
struct volt_ast_node_t {
    volt_ast_node_type_t type;
    const char*          expression_name;  // Which grammar rule created this
    volt_token_t*        token;            // If this is a token node
    volt_vector_t        children;         // Child nodes (volt_ast_node_t*)
    void*                data;             // Extra data specific to node type
};

// Parser state
struct volt_parser_t {
    volt_allocator_t*           allocator;
    volt_vector_t*              tokens;         // Input tokens
    size_t                      current;        // Current token index
    volt_expression_registry_t* registry;       // Grammar rules
    volt_ast_node_t*            root;           // Root AST node
    volt_error_handler_t*       error_handler;  // Parse errors
    const char*                 input_stream_name;
    size_t                      furthest_error_pos;
    char                        furthest_error_msg[256];
    bool                        error_reported;
};

// Parser functions
volt_status_code_t volt_parser_init(volt_parser_t*, volt_allocator_t*, volt_vector_t*,
                                    volt_error_handler_t*, const char*);
volt_status_code_t volt_parser_parse(volt_parser_t*);
volt_status_code_t volt_parser_deinit(volt_parser_t*);

void volt_ast_print_tree(volt_ast_node_t*, int32_t);

// AST functions
volt_ast_node_t* volt_ast_node_create(volt_parser_t*, volt_ast_node_type_t, const char*);
void             volt_ast_node_add_child(volt_ast_node_t*, volt_ast_node_t*);
void             volt_ast_node_free(volt_parser_t*, volt_ast_node_t*);

#ifdef __cplusplus
}
#endif

#endif