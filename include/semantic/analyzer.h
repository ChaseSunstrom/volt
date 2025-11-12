#ifndef VOLT_SEMANTIC_ANALYZER_H
#define VOLT_SEMANTIC_ANALYZER_H

#include <parser/parser.h>
#include <util/memory/allocator.h>
#include <util/types/vector.h>
#include <volt/error.h>

// Forward declarations
typedef struct volt_semantic_analyzer_t volt_semantic_analyzer_t;
typedef struct volt_symbol_t            volt_symbol_t;
typedef struct volt_scope_t             volt_scope_t;
typedef struct volt_type_info_t         volt_type_info_t;

// Symbol kinds
typedef enum {
    VOLT_SYMBOL_VARIABLE,
    VOLT_SYMBOL_FUNCTION,
    VOLT_SYMBOL_TYPE,
    VOLT_SYMBOL_NAMESPACE,
    VOLT_SYMBOL_ENUM_VARIANT,
    VOLT_SYMBOL_GENERIC_PARAM,
} volt_symbol_kind_t;

// Type kinds
typedef enum {
    VOLT_TYPE_VOID,
    VOLT_TYPE_I8,
    VOLT_TYPE_I16,
    VOLT_TYPE_I32,
    VOLT_TYPE_I64,
    VOLT_TYPE_I128,
    VOLT_TYPE_U8,
    VOLT_TYPE_U16,
    VOLT_TYPE_U32,
    VOLT_TYPE_U64,
    VOLT_TYPE_U128,
    VOLT_TYPE_F16,
    VOLT_TYPE_F32,
    VOLT_TYPE_F64,
    VOLT_TYPE_F128,
    VOLT_TYPE_BOOL,
    VOLT_TYPE_ISIZE,
    VOLT_TYPE_USIZE,
    VOLT_TYPE_CSTR,
    VOLT_TYPE_STR,
    VOLT_TYPE_TYPE,
    VOLT_TYPE_POINTER,
    VOLT_TYPE_REFERENCE,
    VOLT_TYPE_ARRAY,
    VOLT_TYPE_SLICE,
    VOLT_TYPE_TUPLE,
    VOLT_TYPE_STRUCT,
    VOLT_TYPE_ENUM,
    VOLT_TYPE_ERROR,
    VOLT_TYPE_FUNCTION,
    VOLT_TYPE_GENERIC,
    VOLT_TYPE_UNKNOWN,  // For forward references, will be resolved later
} volt_type_kind_t;

// Type information (can be partially filled)
struct volt_type_info_t {
    volt_type_kind_t kind;
    const char*      name;

    // For composite types (filled progressively)
    volt_type_info_t* base_type;      // For pointers, arrays, etc.
    volt_vector_t     element_types;  // For tuples, function params (vector of volt_type_info_t*)
    volt_type_info_t* return_type;    // For functions

    // For structs/enums (filled when we analyze their declaration)
    volt_vector_t fields;    // vector of volt_symbol_t* (struct fields)
    volt_vector_t variants;  // vector of volt_symbol_t* (enum variants)

    // Size and alignment (computed after type is complete)
    size_t size;
    size_t alignment;
    bool   size_computed;

    // Flags
    bool is_const;
    bool is_nullable;
    bool is_complete;  // true when type definition is fully analyzed
};

// Symbol in symbol table
struct volt_symbol_t {
    volt_symbol_kind_t kind;
    const char*        name;
    volt_type_info_t*  type;         // Can be NULL initially, filled later
    volt_ast_node_t*   declaration;  // AST node where this was declared
    volt_scope_t*      scope;        // Scope where this symbol lives

    // For functions
    volt_vector_t parameters;  // vector of volt_symbol_t*
    bool          is_comptime;
    bool          is_async;
    bool          is_extern;

    // For variables
    bool is_mutable;  // true for 'var', false for 'val'
    bool is_static;

    // Source location
    size_t line;
    size_t column;

    // Resolution state
    bool is_resolved;  // true when type is fully resolved
};

// Scope (symbol table)
struct volt_scope_t {
    volt_scope_t* parent;    // Parent scope (NULL for global)
    volt_vector_t symbols;   // vector of volt_symbol_t*
    volt_vector_t children;  // vector of volt_scope_t* (child scopes)

    // Scope type (for break/continue validation)
    enum {
        VOLT_SCOPE_GLOBAL,
        VOLT_SCOPE_FUNCTION,
        VOLT_SCOPE_BLOCK,
        VOLT_SCOPE_LOOP,
        VOLT_SCOPE_MATCH,
    } scope_type;

    // For function scopes
    volt_type_info_t* return_type;  // Can be NULL until analyzed
};

// Semantic analyzer
struct volt_semantic_analyzer_t {
    volt_allocator_t*     allocator;
    volt_error_handler_t* error_handler;

    volt_ast_node_t** asts;              // Array of root ASTs from all parsers
    const char**      input_stream_names; // Array of filenames for error reporting
    size_t            ast_count;          // Number of ASTs
    size_t            current_file_index; // Current file being analyzed (for error reporting)

    volt_scope_t*    global_scope;   // Global symbol table
    volt_scope_t*    current_scope;  // Current scope during analysis

    // Type cache (for built-in types)
    volt_type_info_t* type_void;
    volt_type_info_t* type_i8;
    volt_type_info_t* type_i16;
    volt_type_info_t* type_i32;
    volt_type_info_t* type_i64;
    volt_type_info_t* type_i128;
    volt_type_info_t* type_u8;
    volt_type_info_t* type_u16;
    volt_type_info_t* type_u32;
    volt_type_info_t* type_u64;
    volt_type_info_t* type_u128;
    volt_type_info_t* type_f16;
    volt_type_info_t* type_f32;
    volt_type_info_t* type_f64;
    volt_type_info_t* type_f128;
    volt_type_info_t* type_bool;
    volt_type_info_t* type_isize;
    volt_type_info_t* type_usize;
    volt_type_info_t* type_cstr;
    volt_type_info_t* type_str;
    volt_type_info_t* type_type;
    volt_type_info_t* type_unknown;

    // Unresolved symbols (for forward references)
    volt_vector_t unresolved_symbols;  // vector of volt_symbol_t*

    // Analysis state
    bool   had_error;
    size_t error_count;
};

// Initialize semantic analyzer
volt_status_code_t volt_semantic_analyzer_init(volt_semantic_analyzer_t* analyzer,
                                               volt_allocator_t*         allocator,
                                               volt_ast_node_t**         asts,
                                               const char**              input_stream_names,
                                               size_t                    ast_count,
                                               volt_error_handler_t*     error_handler);

// Run semantic analysis on the AST (multi-pass)
volt_status_code_t volt_semantic_analyzer_analyze(volt_semantic_analyzer_t* analyzer);

// Cleanup
volt_status_code_t volt_semantic_analyzer_deinit(volt_semantic_analyzer_t* analyzer);

// Symbol table operations
volt_scope_t*  volt_scope_create(volt_semantic_analyzer_t* analyzer, volt_scope_t* parent);
volt_symbol_t* volt_scope_lookup(volt_scope_t* scope, const char* name, bool recursive);
volt_symbol_t* volt_scope_insert(volt_semantic_analyzer_t* analyzer, volt_scope_t* scope,
                                 volt_symbol_t* symbol);

// Type operations
volt_type_info_t* volt_type_create(volt_semantic_analyzer_t* analyzer, volt_type_kind_t kind);
volt_type_info_t* volt_type_from_ast(volt_semantic_analyzer_t* analyzer,
                                     volt_ast_node_t*          type_node);
bool              volt_type_equals(volt_type_info_t* a, volt_type_info_t* b);
bool              volt_type_is_numeric(volt_type_info_t* type);
bool              volt_type_is_integer(volt_type_info_t* type);
bool              volt_type_is_floating(volt_type_info_t* type);
const char*       volt_type_to_string(volt_type_info_t* type);

#endif  // VOLT_SEMANTIC_ANALYZER_H
