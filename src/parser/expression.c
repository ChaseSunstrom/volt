#include <parser/expression.h>
#include <parser/parser.h>
#include <pch.h>

// Expression Registry
#define REGISTRY_INITIAL_CAPACITY 64

// TODO: Change all std memory allocations to use allocator

volt_status_code_t volt_expression_registry_init(volt_expression_registry_t* registry,
                                                 volt_allocator_t*           allocator) {
    if (!registry)
        return VOLT_FAILURE;

    registry->allocator = allocator ? allocator : &volt_default_allocator;
    registry->capacity  = REGISTRY_INITIAL_CAPACITY;
    registry->count     = 0;
    registry->expressions =
        registry->allocator->malloc(registry->capacity * sizeof(volt_expression_t*));

    return VOLT_SUCCESS;
}

volt_status_code_t volt_expression_registry_deinit(volt_expression_registry_t* registry) {
    if (!registry)
        return VOLT_FAILURE;

    for (size_t i = 0; i < registry->count; i++) {
        volt_expression_free(registry->expressions[i]);
    }

    registry->allocator->free(registry->expressions);

    return VOLT_SUCCESS;
}

void volt_expression_registry_add(volt_expression_registry_t* registry, volt_expression_t* expr) {
    if (!registry || !expr)
        return;

    if (registry->count >= registry->capacity) {
        size_t              new_capacity = registry->capacity * 2;
        volt_expression_t** new_exprs    = registry->allocator->realloc(
            registry->expressions, new_capacity * sizeof(volt_expression_t*));
        if (!new_exprs)
            return;

        registry->expressions = new_exprs;
        registry->capacity    = new_capacity;
    }

    registry->expressions[registry->count++] = expr;
}

volt_expression_t* volt_expression_registry_get(volt_expression_registry_t* registry,
                                                const char*                 name) {
    if (!registry || !name)
        return NULL;

    for (size_t i = 0; i < registry->count; i++) {
        if (strcmp(registry->expressions[i]->expression_name, name) == 0) {
            return registry->expressions[i];
        }
    }

    return NULL;
}

// Expression Construction - REQUIRED elements
volt_subexpression_t volt_sub_token(volt_token_type_t token_type) {
    return (volt_subexpression_t) {
        .token_type = token_type, .is_subexpression = false, .is_optional = false};
}

volt_subexpression_t volt_sub_expr(const char* name) {
    return (volt_subexpression_t) {
        .subexpression = name, .is_subexpression = true, .is_optional = false};
}

// Expression Construction - OPTIONAL elements
volt_subexpression_t volt_sub_opt_token(volt_token_type_t token_type) {
    return (volt_subexpression_t) {
        .token_type = token_type, .is_subexpression = false, .is_optional = true};
}

volt_subexpression_t volt_sub_opt_expr(const char* name) {
    return (volt_subexpression_t) {
        .subexpression = name, .is_subexpression = true, .is_optional = true};
}

volt_expression_t* volt_expression_create(const char* name) {
    volt_expression_t* expr = malloc(sizeof(volt_expression_t));
    if (!expr)
        return NULL;

    expr->expression_name     = name;
    expr->num_alternatives    = 0;
    expr->capacity            = 4;
    expr->alternatives        = calloc(expr->capacity, sizeof(volt_subexpression_t*));
    expr->alternative_lengths = calloc(expr->capacity, sizeof(size_t));

    if (!expr->alternatives || !expr->alternative_lengths) {
        volt_expression_free(expr);
        return NULL;
    }

    return expr;
}

void volt_expression_add_alt(volt_expression_t* expr, volt_subexpression_t* subs, size_t count) {
    if (!expr)
        return;

    if (expr->num_alternatives >= expr->capacity) {
        size_t new_capacity = expr->capacity * 2;

        volt_subexpression_t** new_alts =
            realloc(expr->alternatives, new_capacity * sizeof(volt_subexpression_t*));
        size_t* new_lengths = realloc(expr->alternative_lengths, new_capacity * sizeof(size_t));

        if (!new_alts || !new_lengths)
            return;

        expr->alternatives        = new_alts;
        expr->alternative_lengths = new_lengths;
        expr->capacity            = new_capacity;

        memset(expr->alternatives + expr->num_alternatives, 0,
               (new_capacity - expr->num_alternatives) * sizeof(volt_subexpression_t*));
        memset(expr->alternative_lengths + expr->num_alternatives, 0,
               (new_capacity - expr->num_alternatives) * sizeof(size_t));
    }

    expr->alternatives[expr->num_alternatives] = malloc(count * sizeof(volt_subexpression_t));
    if (!expr->alternatives[expr->num_alternatives])
        return;

    memcpy(expr->alternatives[expr->num_alternatives], subs, count * sizeof(volt_subexpression_t));
    expr->alternative_lengths[expr->num_alternatives] = count;
    expr->num_alternatives++;
}

void volt_expression_free(volt_expression_t* expr) {
    if (!expr)
        return;

    if (expr->alternatives) {
        for (size_t i = 0; i < expr->num_alternatives; i++) {
            free(expr->alternatives[i]);
        }
        free(expr->alternatives);
    }
    free(expr->alternative_lengths);
    free(expr);
}

void volt_define_expressions(volt_expression_registry_t* registry) {
    volt_expression_t* expr;

    // unit ::= items
    expr = volt_expression_create("unit");
    VOLT_ALT(expr, volt_expr("items"));
    volt_expression_registry_add(registry, expr);

    // items ::= item items_rest | ε
    expr = volt_expression_create("items");
    VOLT_ALT(expr, volt_expr("item"), volt_expr("items_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // items_rest ::= item items_rest | ε
    expr = volt_expression_create("items_rest");
    VOLT_ALT(expr, volt_expr("item"), volt_expr("items_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // item ::= attributes? (extern_decl | export_decl | func_def | ...)
    expr = volt_expression_create("item");
    VOLT_ALT(expr, volt_opt_expr("attributes"), volt_expr("extern_decl"));
    VOLT_ALT(expr, volt_opt_expr("attributes"), volt_expr("export_decl"));
    VOLT_ALT(expr, volt_opt_expr("attributes"), volt_expr("func_def"));
    VOLT_ALT(expr, volt_opt_expr("attributes"), volt_expr("use_decl"));
    VOLT_ALT(expr, volt_opt_expr("attributes"), volt_expr("namespace_decl"));
    VOLT_ALT(expr, volt_opt_expr("attributes"), volt_expr("struct_decl"));
    VOLT_ALT(expr, volt_opt_expr("attributes"), volt_expr("enum_decl"));
    VOLT_ALT(expr, volt_opt_expr("attributes"), volt_expr("error_decl"));
    VOLT_ALT(expr, volt_opt_expr("attributes"), volt_expr("trait_decl"));
    VOLT_ALT(expr, volt_opt_expr("attributes"), volt_expr("attach_decl"));
    volt_expression_registry_add(registry, expr);

    // ATTRIBUTES

    // attributes ::= @ IDENTIFIER LPAREN array_literal RPAREN
    expr = volt_expression_create("attributes");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_AT), volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL),
             volt_token(VOLT_TOKEN_TYPE_LPAREN), volt_expr("array_literal"),
             volt_token(VOLT_TOKEN_TYPE_RPAREN));
    volt_expression_registry_add(registry, expr);

    // use_decl ::= use use_path SEMICOLON
    //           | use LBRACE use_list RBRACE as? IDENTIFIER? SEMICOLON
    expr = volt_expression_create("use_decl");
    // Try C header imports FIRST (more specific)
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_USE_KW), volt_token(VOLT_TOKEN_TYPE_LBRACE),
             volt_expr("string_list"), volt_token(VOLT_TOKEN_TYPE_RBRACE),
             volt_token(VOLT_TOKEN_TYPE_AS_KW),
             volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL),  // namespace name
             volt_token(VOLT_TOKEN_TYPE_SEMICOLON));
    // Normal module import SECOND (less specific)
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_USE_KW), volt_expr("use_path"),
             volt_token(VOLT_TOKEN_TYPE_SEMICOLON));
    volt_expression_registry_add(registry, expr);

    // use_path ::= IDENTIFIER use_path_rest
    expr = volt_expression_create("use_path");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL), volt_expr("use_path_rest"));
    volt_expression_registry_add(registry, expr);

    // use_path_rest ::= :: IDENTIFIER use_path_rest | ε
    expr = volt_expression_create("use_path_rest");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_COLON_COLON),
             volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL), volt_expr("use_path_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // string_list ::= STRING_LITERAL string_list_rest
    expr = volt_expression_create("string_list");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_STRING_LITERAL), volt_expr("string_list_rest"));
    volt_expression_registry_add(registry, expr);

    // string_list_rest ::= COMMA STRING_LITERAL string_list_rest | ε
    expr = volt_expression_create("string_list_rest");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_COMMA), volt_token(VOLT_TOKEN_TYPE_STRING_LITERAL),
             volt_expr("string_list_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // namespace_decl ::= namespace namespace_path LBRACE items RBRACE
    expr = volt_expression_create("namespace_decl");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_NAMESPACE_KW), volt_expr("namespace_path"),
             volt_token(VOLT_TOKEN_TYPE_LBRACE), volt_expr("items"),
             volt_token(VOLT_TOKEN_TYPE_RBRACE));
    volt_expression_registry_add(registry, expr);

    // namespace_path ::= IDENTIFIER namespace_path_rest
    expr = volt_expression_create("namespace_path");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL),
             volt_expr("namespace_path_rest"));
    volt_expression_registry_add(registry, expr);

    // namespace_path_rest ::= :: IDENTIFIER namespace_path_rest | ε
    expr = volt_expression_create("namespace_path_rest");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_COLON_COLON),
             volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL), volt_expr("namespace_path_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // func_def ::= visibility? comptime? async? attach? fn generics? IDENTIFIER
    //              LPAREN params RPAREN error_type? TACK_RANGLE type block
    expr = volt_expression_create("func_def");
    VOLT_ALT(expr, volt_opt_expr("generics"), volt_opt_expr("visibility"),
             volt_opt_token(VOLT_TOKEN_TYPE_COMPTIME_KW), volt_opt_token(VOLT_TOKEN_TYPE_ASYNC_KW),
             volt_opt_token(VOLT_TOKEN_TYPE_ATTACH_KW), volt_token(VOLT_TOKEN_TYPE_FN_KW),
             volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL), volt_token(VOLT_TOKEN_TYPE_LPAREN),
             volt_expr("params"), volt_token(VOLT_TOKEN_TYPE_RPAREN), volt_opt_expr("error_type"),
             volt_token(VOLT_TOKEN_TYPE_TACK_RANGLE), volt_expr("type"), volt_expr("block"));
    volt_expression_registry_add(registry, expr);

    // visibility ::= public | intern
    expr = volt_expression_create("visibility");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_PUBLIC_KW));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_INTERNAL_KW));
    volt_expression_registry_add(registry, expr);

    // error_type ::= type !
    expr = volt_expression_create("error_type");
    VOLT_ALT(expr, volt_expr("type"), volt_token(VOLT_TOKEN_TYPE_BANG));
    volt_expression_registry_add(registry, expr);

    // extern_decl ::= extern STRING_LITERAL? fn IDENTIFIER LPAREN params RPAREN
    //                 error_type? TACK_RANGLE type SEMICOLON
    expr = volt_expression_create("extern_decl");
    VOLT_ALT(expr, volt_opt_expr("generics"), volt_token(VOLT_TOKEN_TYPE_EXTERN_KW),
             volt_opt_token(VOLT_TOKEN_TYPE_STRING_LITERAL), volt_token(VOLT_TOKEN_TYPE_FN_KW),
             volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL), volt_token(VOLT_TOKEN_TYPE_LPAREN),
             volt_expr("params"), volt_token(VOLT_TOKEN_TYPE_RPAREN), volt_opt_expr("error_type"),
             volt_token(VOLT_TOKEN_TYPE_TACK_RANGLE), volt_expr("type"),
             volt_token(VOLT_TOKEN_TYPE_SEMICOLON));
    volt_expression_registry_add(registry, expr);

    // export_decl ::= export STRING_LITERAL? fn IDENTIFIER LPAREN params RPAREN
    //                 error_type? TACK_RANGLE type block
    expr = volt_expression_create("export_decl");
    VOLT_ALT(expr, volt_opt_expr("generics"), volt_token(VOLT_TOKEN_TYPE_EXPORT_KW),
             volt_opt_token(VOLT_TOKEN_TYPE_STRING_LITERAL), volt_token(VOLT_TOKEN_TYPE_FN_KW),
             volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL), volt_token(VOLT_TOKEN_TYPE_LPAREN),
             volt_expr("params"), volt_token(VOLT_TOKEN_TYPE_RPAREN), volt_opt_expr("error_type"),
             volt_token(VOLT_TOKEN_TYPE_TACK_RANGLE), volt_expr("type"), volt_expr("block"));
    volt_expression_registry_add(registry, expr);

    // generics ::= LANGLE generic_params RANGLE (for definitions)
    expr = volt_expression_create("generics");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_LANGLE), volt_expr("generic_params"),
             volt_token(VOLT_TOKEN_TYPE_RANGLE));
    volt_expression_registry_add(registry, expr);

    // generic_args ::= LANGLE type_list RANGLE (for calls/instantiations)
    expr = volt_expression_create("generic_args");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_LANGLE), volt_expr("type_list"),
             volt_token(VOLT_TOKEN_TYPE_RANGLE));
    volt_expression_registry_add(registry, expr);

    // generic_params ::= generic_param generic_params_rest
    expr = volt_expression_create("generic_params");
    VOLT_ALT(expr, volt_expr("generic_param"), volt_expr("generic_params_rest"));
    volt_expression_registry_add(registry, expr);

    // generic_params_rest ::= COMMA generic_param generic_params_rest | ε
    expr = volt_expression_create("generic_params_rest");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_COMMA), volt_expr("generic_param"),
             volt_expr("generic_params_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // generic_param ::= IDENTIFIER COLON type_constraint (EQUAL expression)?
    expr = volt_expression_create("generic_param");
    // WITH default value
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL),
             volt_token(VOLT_TOKEN_TYPE_COLON), volt_expr("type_constraint"),
             volt_token(VOLT_TOKEN_TYPE_EQUAL), volt_expr("expression"));
    // WITHOUT default value
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL),
             volt_token(VOLT_TOKEN_TYPE_COLON), volt_expr("type_constraint"));
    volt_expression_registry_add(registry, expr);

    // type_constraint ::= type | comptime_fn_call | type[]
    expr = volt_expression_create("type_constraint");
    VOLT_ALT(expr, volt_expr("type"));
    VOLT_ALT(expr, volt_expr("comptime_fn_call"));
    VOLT_ALT(expr, volt_expr("type"), volt_token(VOLT_TOKEN_TYPE_LBRACKET),
             volt_token(VOLT_TOKEN_TYPE_RBRACKET));
    volt_expression_registry_add(registry, expr);

    // comptime_fn_call ::= IDENTIFIER LPAREN args RPAREN
    expr = volt_expression_create("comptime_fn_call");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL),
             volt_token(VOLT_TOKEN_TYPE_LPAREN), volt_expr("args"),
             volt_token(VOLT_TOKEN_TYPE_RPAREN));
    volt_expression_registry_add(registry, expr);

    // params ::= param params_rest | IDENTIFIER COLON type LBRACKET RBRACKET | ε
    expr = volt_expression_create("params");
    VOLT_ALT(expr, volt_expr("param"), volt_expr("params_rest"));
    // Variadic: Args: type[]
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL),
             volt_token(VOLT_TOKEN_TYPE_COLON), volt_expr("type"),
             volt_token(VOLT_TOKEN_TYPE_LBRACKET), volt_token(VOLT_TOKEN_TYPE_RBRACKET));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // params_rest ::= COMMA param params_rest | ε
    expr = volt_expression_create("params_rest");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_COMMA), volt_expr("param"), volt_expr("params_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // param ::= static? (IDENTIFIER | this) COLON type (EQUAL expression)? | type | this
    expr = volt_expression_create("param");
    // 'this' parameter with type AND default value
    VOLT_ALT(expr, volt_opt_token(VOLT_TOKEN_TYPE_STATIC_KW), volt_token(VOLT_TOKEN_TYPE_THIS_KW),
             volt_token(VOLT_TOKEN_TYPE_COLON), volt_expr("type"),
             volt_token(VOLT_TOKEN_TYPE_EQUAL), volt_expr("expression"));
    // 'this' parameter with type, NO default value
    VOLT_ALT(expr, volt_opt_token(VOLT_TOKEN_TYPE_STATIC_KW), volt_token(VOLT_TOKEN_TYPE_THIS_KW),
             volt_token(VOLT_TOKEN_TYPE_COLON), volt_expr("type"));
    // Regular identifier parameter WITH default value
    VOLT_ALT(expr, volt_opt_token(VOLT_TOKEN_TYPE_STATIC_KW),
             volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL), volt_token(VOLT_TOKEN_TYPE_COLON),
             volt_expr("type"), volt_token(VOLT_TOKEN_TYPE_EQUAL), volt_expr("expression"));
    // Regular identifier parameter WITHOUT default value
    VOLT_ALT(expr, volt_opt_token(VOLT_TOKEN_TYPE_STATIC_KW),
             volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL), volt_token(VOLT_TOKEN_TYPE_COLON),
             volt_expr("type"));
    // Unnamed 'this' parameter (for trait methods)
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_THIS_KW));
    // Unnamed parameter: just a type (for C-style or generic refs)
    VOLT_ALT(expr, volt_expr("type"));
    volt_expression_registry_add(registry, expr);

    // type ::= base_type type_suffixes?
    expr = volt_expression_create("type");
    VOLT_ALT(expr, volt_expr("base_type"), volt_opt_expr("type_suffixes"));
    volt_expression_registry_add(registry, expr);

    // base_type ::= error_wrapper_type | named_error_wrapper | primitive_type | named_type |
    // tuple_type | closure_type
    expr = volt_expression_create("base_type");
    VOLT_ALT(expr, volt_expr("error_wrapper_type"));   // Try generic error!type first!
    VOLT_ALT(expr, volt_expr("named_error_wrapper"));  // Try named_error!type second!
    VOLT_ALT(expr, volt_expr("primitive_type"));
    VOLT_ALT(expr, volt_expr("named_type"));
    VOLT_ALT(expr, volt_expr("tuple_type"));
    VOLT_ALT(expr, volt_expr("closure_type"));
    volt_expression_registry_add(registry, expr);

    // error_wrapper_type ::= error ! type
    expr = volt_expression_create("error_wrapper_type");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_ERROR_KW), volt_token(VOLT_TOKEN_TYPE_BANG),
             volt_expr("type"));
    volt_expression_registry_add(registry, expr);

    // named_error_wrapper ::= path generic_args? ! type
    expr = volt_expression_create("named_error_wrapper");
    VOLT_ALT(expr, volt_expr("path"), volt_opt_expr("generic_args"),
             volt_token(VOLT_TOKEN_TYPE_BANG), volt_expr("type"));
    volt_expression_registry_add(registry, expr);

    // primitive_type ::= i8|i16|i32|...|str
    expr = volt_expression_create("primitive_type");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_I8_KW));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_I16_KW));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_I32_KW));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_I64_KW));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_I128_KW));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_U8_KW));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_U16_KW));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_U32_KW));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_U64_KW));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_U128_KW));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_F16_KW));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_F32_KW));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_F64_KW));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_F128_KW));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_BOOL_KW));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_ISIZE_KW));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_USIZE_KW));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_TYPE_KW));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_CSTR_KW));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_STR_KW));
    volt_expression_registry_add(registry, expr);

    // named_type ::= !? path generic_args?
    expr = volt_expression_create("named_type");
    VOLT_ALT(expr, volt_opt_token(VOLT_TOKEN_TYPE_BANG), volt_expr("path"),
             volt_opt_expr("generic_args"));
    volt_expression_registry_add(registry, expr);

    // path ::= IDENTIFIER path_rest
    expr = volt_expression_create("path");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL), volt_expr("path_rest"));
    volt_expression_registry_add(registry, expr);

    // path_rest ::= :: IDENTIFIER path_rest | ε
    expr = volt_expression_create("path_rest");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_COLON_COLON),
             volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL), volt_expr("path_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // tuple_type ::= LPAREN type_list RPAREN
    expr = volt_expression_create("tuple_type");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_LPAREN), volt_expr("type_list"),
             volt_token(VOLT_TOKEN_TYPE_RPAREN));
    volt_expression_registry_add(registry, expr);

    // type_list ::= tuple_field type_list_rest | ε
    expr = volt_expression_create("type_list");
    VOLT_ALT(expr, volt_expr("tuple_field"), volt_expr("type_list_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // type_list_rest ::= COMMA tuple_field type_list_rest | ε
    expr = volt_expression_create("type_list_rest");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_COMMA), volt_expr("tuple_field"),
             volt_expr("type_list_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // tuple_field ::= IDENTIFIER COLON type | type
    expr = volt_expression_create("tuple_field");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL),
             volt_token(VOLT_TOKEN_TYPE_COLON), volt_expr("type"));  // Named field
    VOLT_ALT(expr, volt_expr("type"));                               // Unnamed field
    volt_expression_registry_add(registry, expr);

    // closure_type ::= BAR closure_params BAR TACK_RANGLE type
    expr = volt_expression_create("closure_type");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_BAR), volt_expr("closure_params"),
             volt_token(VOLT_TOKEN_TYPE_BAR), volt_token(VOLT_TOKEN_TYPE_TACK_RANGLE),
             volt_expr("type"));
    volt_expression_registry_add(registry, expr);

    // closure_params ::= type_list
    expr = volt_expression_create("closure_params");
    VOLT_ALT(expr, volt_expr("type_list"));
    volt_expression_registry_add(registry, expr);

    // type_suffixes ::= type_suffix type_suffixes_rest
    expr = volt_expression_create("type_suffixes");
    VOLT_ALT(expr, volt_expr("type_suffix"), volt_expr("type_suffixes_rest"));
    volt_expression_registry_add(registry, expr);

    // type_suffixes_rest ::= type_suffix type_suffixes_rest | ε
    expr = volt_expression_create("type_suffixes_rest");
    VOLT_ALT(expr, volt_expr("type_suffix"), volt_expr("type_suffixes_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // type_suffix ::= * | *? | ? | [] | [..] | [expression]
    expr = volt_expression_create("type_suffix");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_STAR));  // Reference
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_STAR),
             volt_token(VOLT_TOKEN_TYPE_QUESTION));        // Pointer
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_QUESTION));  // Optional
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_LBRACKET),
             volt_token(VOLT_TOKEN_TYPE_RBRACKET));  // Array
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_LBRACKET), volt_token(VOLT_TOKEN_TYPE_DOT_DOT),
             volt_token(VOLT_TOKEN_TYPE_RBRACKET));  // Slice
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_LBRACKET), volt_expr("expression"),
             volt_token(VOLT_TOKEN_TYPE_RBRACKET));  // Sized array
    volt_expression_registry_add(registry, expr);

    // struct_decl ::= visibility? comptime? struct IDENTIFIER generics? LBRACE fields RBRACE
    expr = volt_expression_create("struct_decl");
    VOLT_ALT(expr, volt_opt_expr("generics"), volt_opt_expr("visibility"),
             volt_opt_token(VOLT_TOKEN_TYPE_COMPTIME_KW), volt_token(VOLT_TOKEN_TYPE_STRUCT_KW),
             volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL), volt_token(VOLT_TOKEN_TYPE_LBRACE),
             volt_expr("fields"), volt_token(VOLT_TOKEN_TYPE_RBRACE));
    // Empty struct
    VOLT_ALT(expr, volt_opt_expr("generics"), volt_opt_expr("visibility"),
             volt_opt_token(VOLT_TOKEN_TYPE_COMPTIME_KW), volt_token(VOLT_TOKEN_TYPE_STRUCT_KW),
             volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL), volt_token(VOLT_TOKEN_TYPE_SEMICOLON));
    volt_expression_registry_add(registry, expr);

    // fields ::= field fields_rest | ε
    expr = volt_expression_create("fields");
    VOLT_ALT(expr, volt_expr("field"), volt_expr("fields_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // fields_rest ::= field fields_rest | ε
    expr = volt_expression_create("fields_rest");
    VOLT_ALT(expr, volt_expr("field"), volt_expr("fields_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // field ::= IDENTIFIER COLON type (EQUAL expression)? SEMICOLON
    expr = volt_expression_create("field");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL),
             volt_token(VOLT_TOKEN_TYPE_COLON), volt_expr("type"),
             volt_opt_token(VOLT_TOKEN_TYPE_EQUAL), volt_opt_expr("expression"),
             volt_token(VOLT_TOKEN_TYPE_SEMICOLON));
    volt_expression_registry_add(registry, expr);

    // enum_decl ::= visibility? generics? enum IDENTIFIER LBRACE enum_variants RBRACE
    expr = volt_expression_create("enum_decl");
    VOLT_ALT(expr, volt_opt_expr("generics"), volt_opt_expr("visibility"),
             volt_opt_expr("generics"), volt_token(VOLT_TOKEN_TYPE_ENUM_KW),
             volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL), volt_token(VOLT_TOKEN_TYPE_LBRACE),
             volt_expr("enum_variants"), volt_token(VOLT_TOKEN_TYPE_RBRACE));
    volt_expression_registry_add(registry, expr);

    // enum_variants ::= enum_variant enum_variants_rest
    expr = volt_expression_create("enum_variants");
    VOLT_ALT(expr, volt_expr("enum_variant"), volt_expr("enum_variants_rest"));
    volt_expression_registry_add(registry, expr);

    // enum_variants_rest ::= COMMA enum_variant enum_variants_rest | COMMA | ε
    expr = volt_expression_create("enum_variants_rest");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_COMMA), volt_expr("enum_variant"),
             volt_expr("enum_variants_rest"));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_COMMA));  // Trailing comma
    VOLT_ALT(expr);                                     // Empty
    volt_expression_registry_add(registry, expr);

    // enum_variant ::= IDENTIFIER (COLON type)?
    expr = volt_expression_create("enum_variant");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL),
             volt_opt_token(VOLT_TOKEN_TYPE_COLON), volt_opt_expr("type"));
    volt_expression_registry_add(registry, expr);

    // error_decl ::= visibility? generics? error IDENTIFIER LBRACE enum_variants RBRACE
    expr = volt_expression_create("error_decl");
    VOLT_ALT(expr, volt_opt_expr("generics"), volt_opt_expr("visibility"),
             volt_opt_expr("generics"), volt_token(VOLT_TOKEN_TYPE_ERROR_KW),
             volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL), volt_token(VOLT_TOKEN_TYPE_LBRACE),
             volt_expr("enum_variants"), volt_token(VOLT_TOKEN_TYPE_RBRACE));
    volt_expression_registry_add(registry, expr);

    // TRAIT DECLARATIONS

    // trait_decl ::= visibility? trait IDENTIFIER LBRACE trait_items RBRACE
    expr = volt_expression_create("trait_decl");
    VOLT_ALT(expr, volt_opt_expr("generics"), volt_opt_expr("visibility"),
             volt_token(VOLT_TOKEN_TYPE_TRAIT_KW), volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL),
             volt_token(VOLT_TOKEN_TYPE_LBRACE), volt_expr("trait_items"),
             volt_token(VOLT_TOKEN_TYPE_RBRACE));
    volt_expression_registry_add(registry, expr);

    // trait_items ::= trait_item trait_items_rest | ε
    expr = volt_expression_create("trait_items");
    VOLT_ALT(expr, volt_expr("trait_item"), volt_expr("trait_items_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // trait_items_rest ::= trait_item trait_items_rest | ε
    expr = volt_expression_create("trait_items_rest");
    VOLT_ALT(expr, volt_expr("trait_item"), volt_expr("trait_items_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // trait_item ::= generics? fn generics? IDENTIFIER LPAREN params RPAREN TACK_RANGLE type
    // SEMICOLON
    expr = volt_expression_create("trait_item");
    VOLT_ALT(expr, volt_opt_expr("generics"), volt_token(VOLT_TOKEN_TYPE_FN_KW),
             volt_opt_expr("generics"), volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL),
             volt_token(VOLT_TOKEN_TYPE_LPAREN), volt_expr("params"),
             volt_token(VOLT_TOKEN_TYPE_RPAREN), volt_token(VOLT_TOKEN_TYPE_TACK_RANGLE),
             volt_expr("type"), volt_token(VOLT_TOKEN_TYPE_SEMICOLON));
    volt_expression_registry_add(registry, expr);

    // attach_decl ::= generics? attach trait_path TACK_RANGLE type LBRACE items RBRACE
    expr = volt_expression_create("attach_decl");
    VOLT_ALT(expr, volt_opt_expr("generics"), volt_token(VOLT_TOKEN_TYPE_ATTACH_KW),
             volt_expr("path"), volt_token(VOLT_TOKEN_TYPE_TACK_RANGLE), volt_expr("type"),
             volt_token(VOLT_TOKEN_TYPE_LBRACE), volt_expr("items"),
             volt_token(VOLT_TOKEN_TYPE_RBRACE));
    volt_expression_registry_add(registry, expr);

    // block ::= LBRACE statements RBRACE
    expr = volt_expression_create("block");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_LBRACE), volt_expr("statements"),
             volt_token(VOLT_TOKEN_TYPE_RBRACE));
    volt_expression_registry_add(registry, expr);

    // statements ::= statement statements_rest | ε
    expr = volt_expression_create("statements");
    VOLT_ALT(expr, volt_expr("statement"), volt_expr("statements_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // statements_rest ::= statement statements_rest | ε
    expr = volt_expression_create("statements_rest");
    VOLT_ALT(expr, volt_expr("statement"), volt_expr("statements_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // var_decl ::= comptime? var IDENTIFIER (COLON type)? (EQUAL expression)? SEMICOLON
    expr = volt_expression_create("var_decl");
    VOLT_ALT(expr, volt_opt_token(VOLT_TOKEN_TYPE_COMPTIME_KW), volt_token(VOLT_TOKEN_TYPE_VAR_KW),
             volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL), volt_opt_token(VOLT_TOKEN_TYPE_COLON),
             volt_opt_expr("type"), volt_opt_token(VOLT_TOKEN_TYPE_EQUAL),
             volt_opt_expr("expression"), volt_token(VOLT_TOKEN_TYPE_SEMICOLON));
    volt_expression_registry_add(registry, expr);

    // val_decl ::= comptime? val IDENTIFIER (COLON type)? EQUAL expression SEMICOLON
    expr = volt_expression_create("val_decl");
    VOLT_ALT(expr, volt_opt_token(VOLT_TOKEN_TYPE_COMPTIME_KW), volt_token(VOLT_TOKEN_TYPE_VAL_KW),
             volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL), volt_opt_token(VOLT_TOKEN_TYPE_COLON),
             volt_opt_expr("type"), volt_token(VOLT_TOKEN_TYPE_EQUAL), volt_expr("expression"),
             volt_token(VOLT_TOKEN_TYPE_SEMICOLON));
    volt_expression_registry_add(registry, expr);

    // static_decl ::= static IDENTIFIER COLON type EQUAL expression SEMICOLON
    expr = volt_expression_create("static_decl");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_STATIC_KW),
             volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL), volt_token(VOLT_TOKEN_TYPE_COLON),
             volt_expr("type"), volt_token(VOLT_TOKEN_TYPE_EQUAL), volt_expr("expression"),
             volt_token(VOLT_TOKEN_TYPE_SEMICOLON));
    volt_expression_registry_add(registry, expr);

    // return_stmt ::= return expression? SEMICOLON
    expr = volt_expression_create("return_stmt");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_RETURN_KW), volt_opt_expr("expression"),
             volt_token(VOLT_TOKEN_TYPE_SEMICOLON));
    volt_expression_registry_add(registry, expr);

    // break_stmt ::= break (COLON IDENTIFIER)? SEMICOLON
    expr = volt_expression_create("break_stmt");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_BREAK_KW), volt_opt_token(VOLT_TOKEN_TYPE_COLON),
             volt_opt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL),
             volt_token(VOLT_TOKEN_TYPE_SEMICOLON));
    volt_expression_registry_add(registry, expr);

    expr = volt_expression_create("defer_stmt");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_DEFER_KW), volt_expr("expression"),
             volt_token(VOLT_TOKEN_TYPE_SEMICOLON));
    volt_expression_registry_add(registry, expr);

    // continue_stmt ::= continue (COLON IDENTIFIER)? SEMICOLON
    expr = volt_expression_create("continue_stmt");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_CONTINUE_KW), volt_opt_token(VOLT_TOKEN_TYPE_COLON),
             volt_opt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL),
             volt_token(VOLT_TOKEN_TYPE_SEMICOLON));
    volt_expression_registry_add(registry, expr);

    // suspend_stmt ::= suspend SEMICOLON
    expr = volt_expression_create("suspend_stmt");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_SUSPEND_KW), volt_token(VOLT_TOKEN_TYPE_SEMICOLON));
    volt_expression_registry_add(registry, expr);

    // resume_stmt ::= resume expression SEMICOLON
    expr = volt_expression_create("resume_stmt");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_RESUME_KW), volt_expr("expression"),
             volt_token(VOLT_TOKEN_TYPE_SEMICOLON));
    volt_expression_registry_add(registry, expr);

    // if_stmt ::= comptime? if LPAREN expression RPAREN block (else (if_stmt | block))?
    expr = volt_expression_create("if_stmt");
    VOLT_ALT(expr, volt_opt_token(VOLT_TOKEN_TYPE_COMPTIME_KW), volt_token(VOLT_TOKEN_TYPE_IF_KW),
             volt_token(VOLT_TOKEN_TYPE_LPAREN), volt_expr("expression"),
             volt_token(VOLT_TOKEN_TYPE_RPAREN), volt_expr("block"),
             volt_opt_token(VOLT_TOKEN_TYPE_ELSE_KW), volt_opt_expr("else_clause"));
    volt_expression_registry_add(registry, expr);

    // else_clause ::= if_stmt | block
    expr = volt_expression_create("else_clause");
    VOLT_ALT(expr, volt_expr("if_stmt"));
    VOLT_ALT(expr, volt_expr("block"));
    volt_expression_registry_add(registry, expr);

    // while_stmt ::= label? while LPAREN expression RPAREN block
    expr = volt_expression_create("while_stmt");
    VOLT_ALT(expr, volt_opt_expr("label"), volt_token(VOLT_TOKEN_TYPE_WHILE_KW),
             volt_token(VOLT_TOKEN_TYPE_LPAREN), volt_expr("expression"),
             volt_token(VOLT_TOKEN_TYPE_RPAREN), volt_expr("block"));
    volt_expression_registry_add(registry, expr);

    // loop_stmt ::= label? loop block
    expr = volt_expression_create("loop_stmt");
    VOLT_ALT(expr, volt_opt_expr("label"), volt_token(VOLT_TOKEN_TYPE_LOOP_KW), volt_expr("block"));
    volt_expression_registry_add(registry, expr);

    expr = volt_expression_create("identifier_list");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL),
             volt_expr("identifier_list_rest"));
    volt_expression_registry_add(registry, expr);

    // identifier_list_rest ::= COMMA IDENTIFIER identifier_list_rest | ε
    expr = volt_expression_create("identifier_list_rest");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_COMMA),
             volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL), volt_expr("identifier_list_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // for_iterable_expr - restricted expression that doesn't consume | operator
    // This is used for the expression after 'in' in for loops
    // It stops before bitwise_or to avoid consuming the | in | expr |
    expr = volt_expression_create("for_iterable_expr");
    VOLT_ALT(expr, volt_expr("bitwise_xor_expr"));
    volt_expression_registry_add(registry, expr);

    // for_stmt ::= label? for LPAREN for_binding RPAREN in expression
    //              (BAR expression BAR)? for_captures? block
    // Split into two alternatives: with and without for_pre_expr
    // Try the more specific one (with for_pre_expr) FIRST
    expr = volt_expression_create("for_stmt");
    // Alternative 1: WITH for_pre_expr (more specific - try first)
    VOLT_ALT(expr, volt_opt_expr("label"), volt_token(VOLT_TOKEN_TYPE_FOR_KW),
             volt_expr("for_binding"), volt_token(VOLT_TOKEN_TYPE_IN_KW),
             volt_expr("for_iterable_expr"),  // Use restricted expression
             volt_expr("for_pre_expr"),       // REQUIRED (not optional)
             volt_opt_expr("for_captures"), volt_expr("block"));
    // Alternative 2: WITHOUT for_pre_expr (less specific - try second)
    VOLT_ALT(expr, volt_opt_expr("label"), volt_token(VOLT_TOKEN_TYPE_FOR_KW),
             volt_expr("for_binding"), volt_token(VOLT_TOKEN_TYPE_IN_KW),
             volt_expr("expression"),  // Full expression allowed when no | expr |
             // NO for_pre_expr
             volt_opt_expr("for_captures"), volt_expr("block"));
    volt_expression_registry_add(registry, expr);

    // for_pre_expr ::= BAR expression BAR
    expr = volt_expression_create("for_pre_expr");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_BAR), volt_expr("expression"),
             volt_token(VOLT_TOKEN_TYPE_BAR));
    volt_expression_registry_add(registry, expr);

    // for_binding ::= IDENTIFIER | LPAREN identifier_list RPAREN
    expr = volt_expression_create("for_binding");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_LPAREN), volt_expr("identifier_list"),
             volt_token(VOLT_TOKEN_TYPE_RPAREN));
    volt_expression_registry_add(registry, expr);

    // for_captures ::= LBRACKET capture_list RBRACKET
    expr = volt_expression_create("for_captures");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_LBRACKET), volt_expr("capture_list"),
             volt_token(VOLT_TOKEN_TYPE_RBRACKET));
    volt_expression_registry_add(registry, expr);

    // capture_list ::= capture capture_list_rest
    expr = volt_expression_create("capture_list");
    VOLT_ALT(expr, volt_expr("capture"), volt_expr("capture_list_rest"));
    volt_expression_registry_add(registry, expr);

    // capture_list_rest ::= COMMA capture capture_list_rest | ε
    expr = volt_expression_create("capture_list_rest");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_COMMA), volt_expr("capture"),
             volt_expr("capture_list_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // capture ::= var IDENTIFIER (COLON type)?
    expr = volt_expression_create("capture");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_VAR_KW),
             volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL), volt_opt_token(VOLT_TOKEN_TYPE_COLON),
             volt_opt_expr("type"));
    volt_expression_registry_add(registry, expr);

    // identifier_list ::= IDENTIFIER identifier_list_rest

    // label ::= COLON IDENTIFIER
    expr = volt_expression_create("label");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_COLON),
             volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL));
    volt_expression_registry_add(registry, expr);

    // match_stmt ::= comptime? match expression LBRACE match_arms RBRACE
    expr = volt_expression_create("match_stmt");
    VOLT_ALT(expr, volt_opt_token(VOLT_TOKEN_TYPE_COMPTIME_KW),
             volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL),  // "match" keyword
             volt_expr("expression"), volt_token(VOLT_TOKEN_TYPE_LBRACE), volt_expr("match_arms"),
             volt_token(VOLT_TOKEN_TYPE_RBRACE));
    volt_expression_registry_add(registry, expr);

    // match_arms ::= match_arm match_arms_rest
    expr = volt_expression_create("match_arms");
    VOLT_ALT(expr, volt_expr("match_arm"), volt_expr("match_arms_rest"));
    volt_expression_registry_add(registry, expr);

    // match_arms_rest ::= match_arm match_arms_rest | ε
    expr = volt_expression_create("match_arms_rest");
    VOLT_ALT(expr, volt_expr("match_arm"), volt_expr("match_arms_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // match_arm ::= match_pattern EQUAL_RANGLE (expression SEMICOLON | block SEMICOLON? )
    expr = volt_expression_create("match_arm");
    VOLT_ALT(expr, volt_expr("match_pattern"), volt_token(VOLT_TOKEN_TYPE_EQUAL_RANGLE),
             volt_expr("expression"), volt_token(VOLT_TOKEN_TYPE_SEMICOLON));
    VOLT_ALT(expr, volt_expr("match_pattern"), volt_token(VOLT_TOKEN_TYPE_EQUAL_RANGLE),
             volt_expr("block"), volt_token(VOLT_TOKEN_TYPE_SEMICOLON));  // Block with semicolon
    VOLT_ALT(expr, volt_expr("match_pattern"), volt_token(VOLT_TOKEN_TYPE_EQUAL_RANGLE),
             volt_expr("block"));  // Block without semicolon
    volt_expression_registry_add(registry, expr);

    // match_pattern ::= expression | default
    expr = volt_expression_create("match_pattern");
    VOLT_ALT(expr, volt_expr("expression"));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL));  // "default"
    volt_expression_registry_add(registry, expr);

    // try_catch ::= expression catch (BAR IDENTIFIER BAR)? block
    expr = volt_expression_create("try_catch");
    VOLT_ALT(expr, volt_expr("expression"), volt_token(VOLT_TOKEN_TYPE_CATCH_KW),
             volt_opt_token(VOLT_TOKEN_TYPE_BAR),
             volt_opt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL),
             volt_opt_token(VOLT_TOKEN_TYPE_BAR), volt_expr("block"));
    volt_expression_registry_add(registry, expr);

    // expr_stmt ::= expression SEMICOLON
    expr = volt_expression_create("expr_stmt");
    VOLT_ALT(expr, volt_expr("expression"), volt_token(VOLT_TOKEN_TYPE_SEMICOLON));
    volt_expression_registry_add(registry, expr);

    // EXPRESSIONS (All Fixed for Left Recursion)

    // expression ::= assignment_expr
    expr = volt_expression_create("expression");
    VOLT_ALT(expr, volt_expr("assignment_expr"));
    volt_expression_registry_add(registry, expr);

    // assignment_expr ::= logical_or_expr assignment_expr_rest
    expr = volt_expression_create("assignment_expr");
    VOLT_ALT(expr, volt_expr("logical_or_expr"), volt_expr("assignment_expr_rest"));
    volt_expression_registry_add(registry, expr);

    // assignment_expr_rest ::= assign_op assignment_expr | ε
    expr = volt_expression_create("assignment_expr_rest");
    VOLT_ALT(expr, volt_expr("assign_op"), volt_expr("assignment_expr"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // assign_op ::= = | += | -= | *= | /= | %= | &= | |= | ^= | <<= | >>=
    expr = volt_expression_create("assign_op");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_EQUAL));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_PLUS_EQUAL));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_TACK_EQUAL));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_STAR_EQUAL));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_SLASH_EQUAL));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_PERCENT_EQUAL));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_AMPERSAND_EQUAL));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_BAR_EQUAL));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_CARET_EQUAL));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_LANGLE_LANGLE_EQUAL));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_RANGLE_RANGLE_EQUAL));
    volt_expression_registry_add(registry, expr);

    // logical_or_expr ::= logical_and_expr logical_or_expr_rest
    expr = volt_expression_create("logical_or_expr");
    VOLT_ALT(expr, volt_expr("logical_and_expr"), volt_expr("logical_or_expr_rest"));
    volt_expression_registry_add(registry, expr);

    // logical_or_expr_rest ::= || logical_and_expr logical_or_expr_rest | ε
    expr = volt_expression_create("logical_or_expr_rest");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_BAR_BAR), volt_expr("logical_and_expr"),
             volt_expr("logical_or_expr_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // logical_and_expr ::= bitwise_or_expr logical_and_expr_rest
    expr = volt_expression_create("logical_and_expr");
    VOLT_ALT(expr, volt_expr("bitwise_or_expr"), volt_expr("logical_and_expr_rest"));
    volt_expression_registry_add(registry, expr);

    // logical_and_expr_rest ::= && bitwise_or_expr logical_and_expr_rest | ε
    expr = volt_expression_create("logical_and_expr_rest");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_AMPERSAND_AMPERSAND), volt_expr("bitwise_or_expr"),
             volt_expr("logical_and_expr_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // bitwise_or_expr ::= bitwise_xor_expr bitwise_or_expr_rest
    expr = volt_expression_create("bitwise_or_expr");
    VOLT_ALT(expr, volt_expr("bitwise_xor_expr"), volt_expr("bitwise_or_expr_rest"));
    volt_expression_registry_add(registry, expr);

    // bitwise_or_expr_rest ::= | bitwise_xor_expr bitwise_or_expr_rest | ε
    expr = volt_expression_create("bitwise_or_expr_rest");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_BAR), volt_expr("bitwise_xor_expr"),
             volt_expr("bitwise_or_expr_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // bitwise_xor_expr ::= bitwise_and_expr bitwise_xor_expr_rest
    expr = volt_expression_create("bitwise_xor_expr");
    VOLT_ALT(expr, volt_expr("bitwise_and_expr"), volt_expr("bitwise_xor_expr_rest"));
    volt_expression_registry_add(registry, expr);

    // bitwise_xor_expr_rest ::= ^ bitwise_and_expr bitwise_xor_expr_rest | ε
    expr = volt_expression_create("bitwise_xor_expr_rest");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_CARET), volt_expr("bitwise_and_expr"),
             volt_expr("bitwise_xor_expr_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // bitwise_and_expr ::= equality_expr bitwise_and_expr_rest
    expr = volt_expression_create("bitwise_and_expr");
    VOLT_ALT(expr, volt_expr("equality_expr"), volt_expr("bitwise_and_expr_rest"));
    volt_expression_registry_add(registry, expr);

    // bitwise_and_expr_rest ::= & equality_expr bitwise_and_expr_rest | ε
    expr = volt_expression_create("bitwise_and_expr_rest");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_AMPERSAND), volt_expr("equality_expr"),
             volt_expr("bitwise_and_expr_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // equality_expr ::= relational_expr equality_expr_rest
    expr = volt_expression_create("equality_expr");
    VOLT_ALT(expr, volt_expr("relational_expr"), volt_expr("equality_expr_rest"));
    volt_expression_registry_add(registry, expr);

    // equality_expr_rest ::= (== | !=) relational_expr equality_expr_rest | ε
    expr = volt_expression_create("equality_expr_rest");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_EQUAL_EQUAL), volt_expr("relational_expr"),
             volt_expr("equality_expr_rest"));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_BANG_EQUAL), volt_expr("relational_expr"),
             volt_expr("equality_expr_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // relational_expr ::= shift_expr relational_expr_rest
    expr = volt_expression_create("relational_expr");
    VOLT_ALT(expr, volt_expr("shift_expr"), volt_expr("relational_expr_rest"));
    volt_expression_registry_add(registry, expr);

    // relational_expr_rest ::= (< | > | <= | >=) shift_expr relational_expr_rest | ε
    expr = volt_expression_create("relational_expr_rest");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_LANGLE), volt_expr("shift_expr"),
             volt_expr("relational_expr_rest"));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_RANGLE), volt_expr("shift_expr"),
             volt_expr("relational_expr_rest"));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_LANGLE_EQUAL), volt_expr("shift_expr"),
             volt_expr("relational_expr_rest"));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_RANGLE_EQUAL), volt_expr("shift_expr"),
             volt_expr("relational_expr_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // shift_expr ::= range_expr shift_expr_rest
    expr = volt_expression_create("shift_expr");
    VOLT_ALT(expr, volt_expr("range_expr"), volt_expr("shift_expr_rest"));
    volt_expression_registry_add(registry, expr);

    // shift_expr_rest ::= (<< | >>) range_expr shift_expr_rest | ε
    expr = volt_expression_create("shift_expr_rest");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_LANGLE_LANGLE), volt_expr("range_expr"),
             volt_expr("shift_expr_rest"));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_RANGLE_RANGLE), volt_expr("range_expr"),
             volt_expr("shift_expr_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // range_expr ::= additive_expr range_expr_rest
    expr = volt_expression_create("range_expr");
    VOLT_ALT(expr, volt_expr("additive_expr"), volt_expr("range_expr_rest"));
    volt_expression_registry_add(registry, expr);

    // range_expr_rest ::= (.. | ..=) additive_expr | ε
    expr = volt_expression_create("range_expr_rest");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_DOT_DOT), volt_expr("additive_expr"));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_DOT_DOT_EQUAL), volt_expr("additive_expr"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // additive_expr ::= multiplicative_expr additive_expr_rest
    expr = volt_expression_create("additive_expr");
    VOLT_ALT(expr, volt_expr("multiplicative_expr"), volt_expr("additive_expr_rest"));
    volt_expression_registry_add(registry, expr);

    // additive_expr_rest ::= (+ | -) multiplicative_expr additive_expr_rest | ε
    expr = volt_expression_create("additive_expr_rest");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_PLUS), volt_expr("multiplicative_expr"),
             volt_expr("additive_expr_rest"));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_TACK), volt_expr("multiplicative_expr"),
             volt_expr("additive_expr_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // multiplicative_expr ::= cast_expr multiplicative_expr_rest
    expr = volt_expression_create("multiplicative_expr");
    VOLT_ALT(expr, volt_expr("cast_expr"), volt_expr("multiplicative_expr_rest"));
    volt_expression_registry_add(registry, expr);

    // multiplicative_expr_rest ::= (* | / | %) cast_expr multiplicative_expr_rest | ε
    expr = volt_expression_create("multiplicative_expr_rest");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_STAR), volt_expr("cast_expr"),
             volt_expr("multiplicative_expr_rest"));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_SLASH), volt_expr("cast_expr"),
             volt_expr("multiplicative_expr_rest"));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_PERCENT), volt_expr("cast_expr"),
             volt_expr("multiplicative_expr_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // cast_expr ::= unary_expr cast_expr_rest
    expr = volt_expression_create("cast_expr");
    VOLT_ALT(expr, volt_expr("unary_expr"), volt_expr("cast_expr_rest"));
    volt_expression_registry_add(registry, expr);

    // cast_expr_rest ::= as type | ε
    expr = volt_expression_create("cast_expr_rest");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL),  // "as"
             volt_expr("type"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // unary_expr ::= postfix_expr | unary_op unary_expr | try unary_expr
    expr = volt_expression_create("unary_expr");
    VOLT_ALT(expr, volt_expr("postfix_expr"));
    VOLT_ALT(expr, volt_expr("unary_op"), volt_expr("unary_expr"));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_TRY_KW), volt_expr("unary_expr"));
    volt_expression_registry_add(registry, expr);

    // unary_op ::= - | ! | ~ | * | & | ++ | -- | move | copy
    expr = volt_expression_create("unary_op");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_TACK));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_BANG));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_TILDE));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_STAR));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_AMPERSAND));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_PLUS_PLUS));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_TACK_TACK));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_MOVE_KW));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_COPY_KW));
    volt_expression_registry_add(registry, expr);

    // postfix_expr ::= primary_expr postfix_expr_rest
    expr = volt_expression_create("postfix_expr");
    VOLT_ALT(expr, volt_expr("primary_expr"), volt_expr("postfix_expr_rest"));
    volt_expression_registry_add(registry, expr);

    // postfix_expr_rest ::= postfix_op postfix_expr_rest | ε
    expr = volt_expression_create("postfix_expr_rest");
    VOLT_ALT(expr, volt_expr("postfix_op"), volt_expr("postfix_expr_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // postfix_op ::= call | index | member_access | ++ | -- | catch_clause
    expr = volt_expression_create("postfix_op");
    VOLT_ALT(expr, volt_expr("call"));
    VOLT_ALT(expr, volt_expr("index"));
    VOLT_ALT(expr, volt_expr("member_access"));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_PLUS_PLUS));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_TACK_TACK));
    VOLT_ALT(expr, volt_expr("catch_clause"));  // catch as postfix operator
    volt_expression_registry_add(registry, expr);

    // catch_clause ::= catch (BAR IDENTIFIER BAR)? block
    expr = volt_expression_create("catch_clause");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_CATCH_KW), volt_opt_token(VOLT_TOKEN_TYPE_BAR),
             volt_opt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL),
             volt_opt_token(VOLT_TOKEN_TYPE_BAR), volt_expr("block"));
    volt_expression_registry_add(registry, expr);

    // call ::= generic_args? LPAREN args RPAREN
    expr = volt_expression_create("call");
    VOLT_ALT(expr, volt_opt_expr("generic_args"), volt_token(VOLT_TOKEN_TYPE_LPAREN),
             volt_expr("args"), volt_token(VOLT_TOKEN_TYPE_RPAREN));
    volt_expression_registry_add(registry, expr);

    // args ::= expression args_rest | ε
    expr = volt_expression_create("args");
    VOLT_ALT(expr, volt_expr("expression"), volt_expr("args_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // args_rest ::= COMMA expression args_rest | ε
    expr = volt_expression_create("args_rest");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_COMMA), volt_expr("expression"),
             volt_expr("args_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // index ::= LBRACKET expression RBRACKET
    expr = volt_expression_create("index");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_LBRACKET), volt_expr("expression"),
             volt_token(VOLT_TOKEN_TYPE_RBRACKET));
    volt_expression_registry_add(registry, expr);

    // member_access ::= . (IDENTIFIER | NUMBER) | :: IDENTIFIER | -> IDENTIFIER
    expr = volt_expression_create("member_access");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_DOT), volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_DOT),
             volt_token(VOLT_TOKEN_TYPE_NUMBER_LITERAL));  // Numeric field access for tuples
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_TACK_RANGLE),
             volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_COLON_COLON),
             volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL));
    volt_expression_registry_add(registry, expr);

    // primary_expr ::= literal | identifier | this | builtin | paren_expr |
    //                  struct_literal | array_literal | closure | new_expr |
    //                  error_literal | generic_call | for_stmt | type_scoped_call | primitive_type
    expr = volt_expression_create("primary_expr");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL));
    VOLT_ALT(expr, volt_expr("literal"));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_THIS_KW));
    VOLT_ALT(expr, volt_expr("builtin"));
    VOLT_ALT(expr, volt_expr("paren_expr"));
    VOLT_ALT(expr, volt_expr("struct_literal"));
    VOLT_ALT(expr, volt_expr("array_literal"));
    VOLT_ALT(expr, volt_expr("closure"));
    VOLT_ALT(expr, volt_expr("error_literal"));
    VOLT_ALT(expr, volt_expr("generic_call"));
    VOLT_ALT(expr, volt_expr("for_stmt"));  // For loops as expressions (starts with 'for' keyword)
    VOLT_ALT(expr, volt_expr("type_scoped_call"));  // Type::method() calls
    VOLT_ALT(expr, volt_expr("primitive_type"));    // Type literals (e.g., return i32;)
    volt_expression_registry_add(registry, expr);

    // type_scoped_call ::= primitive_type :: IDENTIFIER generic_args? LPAREN args RPAREN
    expr = volt_expression_create("type_scoped_call");
    VOLT_ALT(expr, volt_expr("primitive_type"), volt_token(VOLT_TOKEN_TYPE_COLON_COLON),
             volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL), volt_opt_expr("generic_args"),
             volt_token(VOLT_TOKEN_TYPE_LPAREN), volt_expr("args"),
             volt_token(VOLT_TOKEN_TYPE_RPAREN));
    volt_expression_registry_add(registry, expr);

    // literal ::= NUMBER | STRING | true | false | null
    expr = volt_expression_create("literal");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_NUMBER_LITERAL));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_STRING_LITERAL));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_TRUE_KW));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_FALSE_KW));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_NULL_KW));
    volt_expression_registry_add(registry, expr);

    // builtin ::= @ IDENTIFIER LPAREN args RPAREN
    //          | @ IDENTIFIER LANGLE type RANGLE LPAREN args RPAREN
    expr = volt_expression_create("builtin");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_AT), volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL),
             volt_token(VOLT_TOKEN_TYPE_LPAREN), volt_expr("args"),
             volt_token(VOLT_TOKEN_TYPE_RPAREN));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_AT), volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL),
             volt_token(VOLT_TOKEN_TYPE_LANGLE), volt_expr("type"),
             volt_token(VOLT_TOKEN_TYPE_RANGLE), volt_token(VOLT_TOKEN_TYPE_LPAREN),
             volt_expr("args"), volt_token(VOLT_TOKEN_TYPE_RPAREN));
    volt_expression_registry_add(registry, expr);

    // paren_expr ::= LPAREN expression RPAREN
    expr = volt_expression_create("paren_expr");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_LPAREN), volt_expr("expression"),
             volt_token(VOLT_TOKEN_TYPE_RPAREN));
    volt_expression_registry_add(registry, expr);

    // struct_literal ::= LBRACE field_inits RBRACE
    expr = volt_expression_create("struct_literal");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_LBRACE), volt_expr("field_inits"),
             volt_token(VOLT_TOKEN_TYPE_RBRACE));
    volt_expression_registry_add(registry, expr);

    // field_inits ::= field_init field_inits_rest | ε
    expr = volt_expression_create("field_inits");
    VOLT_ALT(expr, volt_expr("field_init"), volt_expr("field_inits_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // field_inits_rest ::= COMMA field_init field_inits_rest | ε
    expr = volt_expression_create("field_inits_rest");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_COMMA), volt_expr("field_init"),
             volt_expr("field_inits_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // field_init ::= IDENTIFIER (COLON expression)?
    expr = volt_expression_create("field_init");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL),
             volt_opt_token(VOLT_TOKEN_TYPE_COLON), volt_opt_expr("expression"));
    volt_expression_registry_add(registry, expr);

    // array_literal ::= LBRACKET array_elements RBRACKET | LBRACE array_elements RBRACE
    expr = volt_expression_create("array_literal");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_LBRACKET), volt_expr("array_elements"),
             volt_token(VOLT_TOKEN_TYPE_RBRACKET));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_LBRACE), volt_expr("array_elements"),
             volt_token(VOLT_TOKEN_TYPE_RBRACE));  // C-style curly brace syntax
    volt_expression_registry_add(registry, expr);

    // array_elements ::= expression array_elements_rest | ε
    expr = volt_expression_create("array_elements");
    VOLT_ALT(expr, volt_expr("expression"), volt_expr("array_elements_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // array_elements_rest ::= COMMA expression array_elements_rest | ε
    expr = volt_expression_create("array_elements_rest");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_COMMA), volt_expr("expression"),
             volt_expr("array_elements_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // closure ::= BAR closure_captures BAR (LPAREN params RPAREN)? block
    expr = volt_expression_create("closure");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_BAR), volt_expr("closure_captures"),
             volt_token(VOLT_TOKEN_TYPE_BAR), volt_opt_token(VOLT_TOKEN_TYPE_LPAREN),
             volt_opt_expr("params"), volt_opt_token(VOLT_TOKEN_TYPE_RPAREN), volt_expr("block"));
    volt_expression_registry_add(registry, expr);

    // closure_captures ::= closure_capture closure_captures_rest | ε
    expr = volt_expression_create("closure_captures");
    VOLT_ALT(expr, volt_expr("closure_capture"), volt_expr("closure_captures_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // closure_captures_rest ::= COMMA closure_capture closure_captures_rest | ε
    expr = volt_expression_create("closure_captures_rest");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_COMMA), volt_expr("closure_capture"),
             volt_expr("closure_captures_rest"));
    VOLT_ALT(expr);  // Empty
    volt_expression_registry_add(registry, expr);

    // closure_capture ::= IDENTIFIER (*)?
    expr = volt_expression_create("closure_capture");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL),
             volt_opt_token(VOLT_TOKEN_TYPE_STAR));
    volt_expression_registry_add(registry, expr);

    // error_literal ::= path :: IDENTIFIER (LPAREN expression RPAREN)?
    //                | error
    expr = volt_expression_create("error_literal");
    VOLT_ALT(expr, volt_expr("path"), volt_token(VOLT_TOKEN_TYPE_COLON_COLON),
             volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL), volt_opt_token(VOLT_TOKEN_TYPE_LPAREN),
             volt_opt_expr("expression"), volt_opt_token(VOLT_TOKEN_TYPE_RPAREN));
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_ERROR_KW));  // "error" keyword for generic error
    volt_expression_registry_add(registry, expr);

    // generic_call ::= IDENTIFIER generic_args call
    expr = volt_expression_create("generic_call");
    VOLT_ALT(expr, volt_token(VOLT_TOKEN_TYPE_IDENTIFIER_LITERAL), volt_expr("generic_args"),
             volt_expr("call"));
    volt_expression_registry_add(registry, expr);

    // statement ::= var_decl | val_decl | static_decl | return_stmt | break_stmt |
    //               continue_stmt | if_stmt | while_stmt | for_stmt | loop_stmt |
    //               match_stmt | suspend_stmt | resume_stmt | expr_stmt
    expr = volt_expression_create("statement");
    VOLT_ALT(expr, volt_expr("var_decl"));
    VOLT_ALT(expr, volt_expr("val_decl"));
    VOLT_ALT(expr, volt_expr("static_decl"));
    VOLT_ALT(expr, volt_expr("return_stmt"));
    VOLT_ALT(expr, volt_expr("break_stmt"));
    VOLT_ALT(expr, volt_expr("continue_stmt"));
    VOLT_ALT(expr, volt_expr("if_stmt"));
    VOLT_ALT(expr, volt_expr("defer_stmt"));
    VOLT_ALT(expr, volt_expr("while_stmt"));
    VOLT_ALT(expr, volt_expr("for_stmt"));
    VOLT_ALT(expr, volt_expr("loop_stmt"));
    VOLT_ALT(expr, volt_expr("match_stmt"));
    VOLT_ALT(expr, volt_expr("suspend_stmt"));
    VOLT_ALT(expr, volt_expr("resume_stmt"));
    VOLT_ALT(expr, volt_expr("expr_stmt"));
    volt_expression_registry_add(registry, expr);
}