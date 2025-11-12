#ifndef __VOLT_EXPRESSION_H__
#define __VOLT_EXPRESSION_H__

#include <lexer/token_type.h>
#include <pch.h>
#include <util/memory/allocator.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct volt_subexpression_t volt_subexpression_t;
struct volt_subexpression_t {
    union {
        volt_token_type_t token_type;
        const char*       subexpression;
    };
    bool is_subexpression;
    bool is_optional;  // marks this element as optional
};

typedef struct volt_expression_t volt_expression_t;
struct volt_expression_t {
    const char*            expression_name;
    size_t                 num_alternatives;
    size_t                 capacity;
    volt_subexpression_t** alternatives;
    size_t*                alternative_lengths;
};

typedef struct volt_expression_registry_t volt_expression_registry_t;
struct volt_expression_registry_t {
    volt_expression_t** expressions;
    size_t              count;
    size_t              capacity;
    volt_allocator_t*   allocator;
};

// Expression management
volt_expression_t* volt_expression_create(const char*);
void               volt_expression_add_alt(volt_expression_t* expr, volt_subexpression_t*, size_t);
void               volt_expression_free(volt_expression_t*);

// Helper functions - REQUIRED elements
volt_subexpression_t volt_sub_token(volt_token_type_t);
volt_subexpression_t volt_sub_expr(const char*);

// Helper functions - OPTIONAL elements
volt_subexpression_t volt_sub_opt_token(volt_token_type_t);
volt_subexpression_t volt_sub_opt_expr(const char*);

// Convenient macros
#define volt_token(type)     volt_sub_token(type)
#define volt_expr(name)      volt_sub_expr(name)
#define volt_opt_token(type) volt_sub_opt_token(type)
#define volt_opt_expr(name)  volt_sub_opt_expr(name)

// Magic macro that automatically counts arguments
#define VOLT_ALT(expr_ptr, ...)                           \
    volt_expression_add_alt(                              \
        expr_ptr, (volt_subexpression_t[]) {__VA_ARGS__}, \
        sizeof((volt_subexpression_t[]) {__VA_ARGS__}) / sizeof(volt_subexpression_t))

// Registry for storing and looking up expressions
volt_status_code_t volt_expression_registry_init(volt_expression_registry_t*, volt_allocator_t*);
volt_status_code_t volt_expression_registry_deinit(volt_expression_registry_t*);
void               volt_expression_registry_add(volt_expression_registry_t*, volt_expression_t*);
volt_expression_t* volt_expression_registry_get(volt_expression_registry_t*, const char*);
void               volt_define_expressions(volt_expression_registry_t*);

#ifdef __cplusplus
}
#endif

#endif