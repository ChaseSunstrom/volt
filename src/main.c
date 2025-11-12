#include <pch.h>
#include <stdlib.h>
#include <volt/volt.h>

#include "util/fmt.h"
#include "util/types/vector.h"
#include "volt/error.h"

int32_t main(int32_t argc, char** argv) {
    volt_cmd_args_t args = {0};
    args.argc            = (uint32_t) argc;
    args.argv            = argv;

    volt_error_handler_t error_handler = {0};

    volt_compiler_t compiler = {0};
    compiler.args            = args;
    compiler.error_handler   = error_handler;
    compiler.allocator       = &volt_default_allocator;

    volt_init(&compiler);
    volt_lex(&compiler);
    volt_parse(&compiler);
    volt_build_ast(&compiler);
    volt_analyze(&compiler);
    volt_compile(&compiler);
    volt_link(&compiler);
    volt_deinit(&compiler);

    volt_fmt_logf(VOLT_FMT_LEVEL_INFO, "Compilation finished.");

    return 0;
}
