#include <lexer/lexer.h>
#include <pch.h>
#include <util/fmt.h>
#include <volt/volt.h>

#include "util/types/types.h"
#include "util/types/vector.h"

// Expects argv like: prog <in1> <in2> ... -o <out1> <out2> ...
static inline char*** _volt_fmt_cmd_args(uint32_t argc, char** argv, size_t* o_input_count,
                                         size_t* o_output_count) {
    (void) argc;
    size_t i = 1;
    while (argv[i] && argv[i][0] != '-')
        i++;
    size_t inputs = (i > 1) ? (i - 1) : 0;

    if (!argv[i] || strcmp(argv[i], "-o") != 0) {
        volt_fmt_logf(VOLT_FMT_LEVEL_ERROR, "Expected -o separating inputs/outputs.");
        exit(EXIT_FAILURE);
    }
    i++;  // skip -o

    size_t out_start = i;
    while (argv[i])
        i++;
    size_t outputs = (i > out_start) ? (i - out_start) : 0;

    if (inputs != outputs) {
        volt_fmt_logf(VOLT_FMT_LEVEL_ERROR, "Incorrect amount of input arguments.");
        exit(EXIT_FAILURE);
    }

    char*** out = malloc(sizeof(char**) * 2);
    out[0]      = malloc(sizeof(char*) * inputs);
    out[1]      = malloc(sizeof(char*) * outputs);
    for (size_t k = 0; k < inputs; k++)
        out[0][k] = argv[1 + k];
    for (size_t k = 0; k < outputs; k++)
        out[1][k] = argv[out_start + k];

    *o_input_count  = inputs;
    *o_output_count = outputs;
    return out;
}

static inline const char* _volt_read_file(const char* path) {
    FILE*    fp;
    uint32_t fsize;
    char*    buffer;

    fp = fopen(path, "rb");  // Open in binary read mode
    if (!fp) {
        volt_fmt_logf(VOLT_FMT_LEVEL_ERROR, "Error reading file: {s}", path);
        return NULL;
    }

    fseek(fp, 0L, SEEK_END);       // Go to end of file
    fsize = (uint32_t) ftell(fp);  // Get file size
    rewind(fp);                    // Go back to beginning

    // Allocate memory for entire content + null terminator
    buffer = (char*) calloc(1, fsize + 1);
    if (!buffer) {
        fclose(fp);
        volt_fmt_logf(VOLT_FMT_LEVEL_ERROR, "Error reading file: {s}", path);
        return NULL;
    }

    // Read file into buffer
    if (fread(buffer, fsize, 1, fp) != 1) {
        fclose(fp);
        free(buffer);
        volt_fmt_logf(VOLT_FMT_LEVEL_ERROR, "Error reading file: {s}", path);
        return NULL;
    }

    fclose(fp);

    return (const char*) buffer;
}

int32_t volt_cmd_args_init(volt_cmd_args_t* args, uint32_t argc, char** argv) {
    args->argc = argc;
    args->argv = argv;
    args->parsed_args =
        (const char***) _volt_fmt_cmd_args(argc, argv, &args->input_count, &args->output_count);
    args->input_files  = args->parsed_args[0];  // this is technically unsafe but idgaf
    args->output_files = args->parsed_args[1];
    return VOLT_SUCCESS;
}

int32_t volt_cmd_args_deinit(volt_cmd_args_t* args) {
    free(args->input_files);
    free(args->output_files);
    free(args->parsed_args);

    return VOLT_SUCCESS;
}

int32_t volt_init(volt_compiler_t* compiler) {
    volt_fmt_logf(VOLT_FMT_LEVEL_INFO, "Initializing voltc...");
    volt_cmd_args_t* args = &compiler->args;

    compiler->lexers = malloc(sizeof(volt_lexer_t) * args->input_count);
    memset(compiler->lexers, 0, sizeof(volt_lexer_t) * args->input_count);

    if (!compiler->lexers)
        return VOLT_FAILURE;

    volt_fmt_logf(VOLT_FMT_LEVEL_INFO, "Input size: {i32}, Output size: {i32}",
                  (int32_t) args->input_count, (int32_t) args->output_count);

    for (size_t i = 0; i < args->input_count; i++) {
        const char* input_file = args->input_files[i];

        volt_lexer_t* lexer = &compiler->lexers[i];
        lexer->input_stream = _volt_read_file(input_file);

        if (!lexer->input_stream) {
            volt_fmt_logf(VOLT_FMT_LEVEL_ERROR, "Failed to read: {s}", input_file);
            return VOLT_FAILURE;
        }

        volt_lexer_init(lexer);
        volt_fmt_logf(VOLT_FMT_LEVEL_INFO, "File contents: {s}", lexer->input_stream);
    }
    return VOLT_SUCCESS;
}

int32_t volt_deinit(volt_compiler_t* compiler) {
    // Deinit lexers first (they may own buffers)
    for (size_t i = 0; i < compiler->args.input_count; i++) {
        volt_lexer_t* lexer = &compiler->lexers[i];
        volt_lexer_deinit(lexer);
    }
    free(compiler->lexers);
    compiler->lexers = NULL;

    volt_cmd_args_deinit(&compiler->args);
    return VOLT_SUCCESS;
}

int32_t volt_lex(volt_compiler_t* compiler) {
    (void) compiler;

    for (size_t i = 0; i < compiler->args.input_count; i++) {
    }

    return VOLT_SUCCESS;
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
