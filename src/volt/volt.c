#include <pch.h>
#include <volt/volt.h>

#include "util/fmt.h"

static inline char*** _volt_fmt_cmd_args(uint32_t argc, char** argv) {
    (void) argc;
    const uint32_t offset      = 1;
    uint32_t       input_size  = offset;  // 1 because argv[0] is name of the exe
    uint32_t       output_size = offset;  // 1 because of -o param

    while (argv[input_size] && argv[input_size][0] != '-') {
        input_size++;
    }

    while (argv[input_size + output_size]) {
        output_size++;
    }

    if (input_size != output_size) {
        volt_fmt_logf(VOLT_FMT_LEVEL_ERROR, "Incorrect amount of input arguments.");
        exit(EXIT_FAILURE);
    }

    char*** output = malloc(sizeof(char**) * 2);
    output[0]      = malloc(sizeof(const char*) * input_size);
    output[1]      = malloc(sizeof(const char*) * output_size);

    char** input_files  = output[0];
    char** output_files = output[1];

    for (uint32_t i = offset; i < input_size; i++) {
        volt_fmt_logf(VOLT_FMT_LEVEL_INFO, "Input file: {s}", argv[i]);
        input_files[i] = argv[i];
    }

    for (uint32_t i = offset; i < output_size; i++) {
        volt_fmt_logf(VOLT_FMT_LEVEL_INFO, "Output file: {s}", argv[input_size + i]);
        output_files[i] = argv[input_size + i];
    }

    return output;
}

int32_t volt_cmd_args_init(volt_cmd_args_t* args, uint32_t argc, char** argv) {
    args->argc         = argc;
    args->argv         = argv;
    args->parsed_args  = (const char***) _volt_fmt_cmd_args(argc, argv);
    args->input_files  = args->parsed_args[0];  // this is technically unsafe but idgaf
    args->output_files = args->parsed_args[1];
    return 0;
}

int32_t volt_cmd_args_deinit(volt_cmd_args_t* args) {
    free(args->input_files);
    free(args->output_files);
    free(args->parsed_args);

    return 0;
}

int32_t volt_init(volt_compiler_t* compiler) {
    (void) compiler;
    volt_fmt_logf(VOLT_FMT_LEVEL_INFO, "Initializing voltc...", 10);
    return 0;
}

int32_t vold_deinit(volt_compiler_t* compiler) {
    volt_cmd_args_deinit(&compiler->args);
    return 0;
}

int32_t volt_lex(volt_compiler_t* compiler) {
    (void) compiler;
    return 1;
}

int32_t volt_parse(volt_compiler_t* compiler) {
    (void) compiler;
    return 1;
}

int32_t volt_build_ast(volt_compiler_t* compiler) {
    (void) compiler;
    return 1;
}

int32_t volt_analyze(volt_compiler_t* compiler) {
    (void) compiler;
    return 1;
}

int32_t volt_compile(volt_compiler_t* compiler) {
    (void) compiler;
    return 1;
}

int32_t volt_link(volt_compiler_t* compiler) {
    (void) compiler;
    return 1;
}
