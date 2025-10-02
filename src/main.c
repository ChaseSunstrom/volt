#include <pch.h>
#include <stdlib.h>
#include <volt/volt.h>

#include "util/fmt.h"
#include "util/types/vector.h"
#include "volt/error.h"

int32_t main(int32_t argc, char** argv) {
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

    /* testing stuff lol
    volt_vector_t vector = volt_vector_default();

    for (size_t i = 0; i < 100; i++) {
        volt_vector_push_back(&vector, (void*) i);
    }

    for (size_t i = 0; i < 100; i++) {
        int32_t j = (int32_t) volt_vector_get(&vector, i);
        volt_fmt_logf(VOLT_FMT_LEVEL_INFO, "Item: {i32}", j);
    }

    volt_vector_deinit(&vector);
    */

    return 0;
}
