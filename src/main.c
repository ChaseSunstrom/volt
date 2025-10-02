#include <pch.h>
#include <stdlib.h>
#include <volt/volt.h>

#include "util/fmt.h"
#include "volt/error.h"

int32_t main(int32_t argc, char** argv) {
    volt_fmt_disable_level(VOLT_FMT_LEVEL_INFO);
    volt_cmd_args_t args = {0};
    volt_cmd_args_init(&args, (uint32_t) argc, argv);

    volt_error_handler_t error_handler = {0};

    volt_compiler_t compiler = {0};
    compiler.args            = args;
    compiler.error_handler   = error_handler;

    volt_init(&compiler);
    volt_lex(&compiler);
    volt_parse(&compiler);
    volt_build_ast(&compiler);
    volt_analyze(&compiler);
    volt_compile(&compiler);
    volt_link(&compiler);
    vold_deinit(&compiler);

    return 0;
}
