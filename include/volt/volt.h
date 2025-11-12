#ifndef __VOLT_VOLT_H__
#define __VOLT_VOLT_H__

#include <lexer/lexer.h>
#include <volt/error.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct volt_cmd_args_t volt_cmd_args_t;
struct volt_cmd_args_t {
    uint32_t          argc;
    char**            argv;
    const char***     parsed_args;
    const char**      input_files;
    const char**      output_files;
    size_t            input_count;
    size_t            output_count;
    volt_allocator_t* allocator;
};

typedef struct volt_compiler_t volt_compiler_t;
struct volt_compiler_t {
    volt_cmd_args_t      args;
    volt_error_handler_t error_handler;
    volt_lexer_t*        lexers;
    volt_allocator_t*    allocator;
};

volt_status_code_t volt_cmd_args_init(volt_cmd_args_t*, volt_allocator_t* allocator);
volt_status_code_t volt_cmd_args_deinit(volt_cmd_args_t*);

volt_status_code_t volt_init(volt_compiler_t*);
volt_status_code_t volt_deinit(volt_compiler_t*);
volt_status_code_t volt_lex(volt_compiler_t*);
volt_status_code_t volt_parse(volt_compiler_t*);
volt_status_code_t volt_build_ast(volt_compiler_t*);
volt_status_code_t volt_analyze(volt_compiler_t*);
volt_status_code_t volt_compile(volt_compiler_t*);
volt_status_code_t volt_link(volt_compiler_t*);

#ifdef __cplusplus
}
#endif

#endif  // __VOLT_VOLT_H__
