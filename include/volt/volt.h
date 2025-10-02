#ifndef __VOLT_VOLT_H__
#define __VOLT_VOLT_H__

#include <volt/error.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct volt_cmd_args_t volt_cmd_args_t;
struct volt_cmd_args_t {
    uint32_t      argc;
    char**        argv;
    const char*** parsed_args;
    const char**  input_files;
    const char**  output_files;
};

typedef struct volt_compiler_t volt_compiler_t;
struct volt_compiler_t {
    volt_cmd_args_t      args;
    volt_error_handler_t error_handler;
};

int32_t volt_cmd_args_init(volt_cmd_args_t*, uint32_t, char**);
int32_t volt_cmd_args_deinit(volt_cmd_args_t*);

int32_t volt_init(volt_compiler_t*);
int32_t vold_deinit(volt_compiler_t*);
int32_t volt_lex(volt_compiler_t*);
int32_t volt_parse(volt_compiler_t*);
int32_t volt_build_ast(volt_compiler_t*);
int32_t volt_analyze(volt_compiler_t*);
int32_t volt_compile(volt_compiler_t*);
int32_t volt_link(volt_compiler_t*);

#ifdef __cplusplus
}
#endif

#endif  // __VOLT_VOLT_H__
