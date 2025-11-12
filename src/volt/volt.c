#include <lexer/lexer.h>
#include <pch.h>
#include <util/fmt.h>
#include <volt/volt.h>

#include "util/types/types.h"
#include "util/types/vector.h"

// Expects argv like: prog <in1> <in2> ... -o <out1> <out2> ...
static inline char*** _volt_fmt_cmd_args(uint32_t argc, char** argv, size_t* o_input_count,
                                         size_t* o_output_count, volt_allocator_t* allocator) {
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

    char*** out = allocator->malloc(sizeof(char**) * 2);
    out[0]      = allocator->malloc(sizeof(char*) * inputs);
    out[1]      = allocator->malloc(sizeof(char*) * outputs);
    for (size_t k = 0; k < inputs; k++)
        out[0][k] = argv[1 + k];
    for (size_t k = 0; k < outputs; k++)
        out[1][k] = argv[out_start + k];

    *o_input_count  = inputs;
    *o_output_count = outputs;
    return out;
}

static inline const char* _volt_read_file(const char* path, volt_allocator_t* allocator) {
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
    buffer = (char*) allocator->malloc(fsize + 1);
    if (!buffer) {
        fclose(fp);
        volt_fmt_logf(VOLT_FMT_LEVEL_ERROR, "Error reading file: {s}", path);
        return NULL;
    }

    // Read file into buffer
    if (fread(buffer, fsize, 1, fp) != 1) {
        fclose(fp);
        allocator->free(buffer);
        volt_fmt_logf(VOLT_FMT_LEVEL_ERROR, "Error reading file: {s}", path);
        return NULL;
    }

    fclose(fp);

    return (const char*) buffer;
}

volt_status_code_t volt_cmd_args_init(volt_cmd_args_t* args, volt_allocator_t* allocator) {
    args->allocator   = allocator;
    args->parsed_args = (const char***) _volt_fmt_cmd_args(
        args->argc, args->argv, &args->input_count, &args->output_count, allocator);
    args->input_files  = args->parsed_args[0];  // this is technically unsafe but idgaf
    args->output_files = args->parsed_args[1];
    return VOLT_SUCCESS;
}

volt_status_code_t volt_cmd_args_deinit(volt_cmd_args_t* args) {
    args->allocator->free(args->input_files);
    args->allocator->free(args->output_files);
    args->allocator->free(args->parsed_args);

    return VOLT_SUCCESS;
}

volt_status_code_t volt_init(volt_compiler_t* compiler) {
    if (!compiler->allocator) {
        compiler->allocator = &volt_default_allocator;
    }

    volt_cmd_args_init(&compiler->args, compiler->allocator);
    volt_error_handler_init(&compiler->error_handler, compiler->allocator);

    volt_fmt_logf(VOLT_FMT_LEVEL_INFO, "Initializing voltc...");
    volt_cmd_args_t* args = &compiler->args;

    compiler->lexers = compiler->allocator->malloc(sizeof(volt_lexer_t) * args->input_count);
    memset(compiler->lexers, 0, sizeof(volt_lexer_t) * args->input_count);

    compiler->parsers = compiler->allocator->malloc(sizeof(volt_parser_t) * args->input_count);
    memset(compiler->parsers, 0, sizeof(volt_parser_t) * args->input_count);

    if (!compiler->lexers)
        return VOLT_FAILURE;

    if (!compiler->parsers)
        return VOLT_FAILURE;

    volt_fmt_logf(VOLT_FMT_LEVEL_INFO, "Input size: {i32}, Output size: {i32}",
                  (int32_t) args->input_count, (int32_t) args->output_count);

    return VOLT_SUCCESS;
}

volt_status_code_t volt_deinit(volt_compiler_t* compiler) {
    volt_error_handler_print(&compiler->error_handler);

    // Deinit semantic analyzer
    volt_semantic_analyzer_deinit(&compiler->analyzer);

    // Deinit lexers first (they own buffers)
    for (size_t i = 0; i < compiler->args.input_count; i++) {
        volt_lexer_t* lexer = &compiler->lexers[i];
        volt_lexer_deinit(lexer);
    }

    for (size_t i = 0; i < compiler->args.input_count; i++) {
        volt_parser_t* parser = &compiler->parsers[i];
        volt_parser_deinit(parser);
    }

    compiler->allocator->free(compiler->lexers);
    compiler->lexers = NULL;

    volt_cmd_args_deinit(&compiler->args);
    volt_error_handler_deinit(&compiler->error_handler);

    return VOLT_SUCCESS;
}

volt_status_code_t volt_lex(volt_compiler_t* compiler) {
    for (size_t i = 0; i < compiler->args.input_count; i++) {
        const char* input_file = compiler->args.input_files[i];

        volt_lexer_t*  lexer  = &compiler->lexers[i];
        volt_parser_t* parser = &compiler->parsers[i];

        lexer->input_stream      = _volt_read_file(input_file, compiler->allocator);
        lexer->error_handler     = &compiler->error_handler;
        lexer->input_stream_name = input_file;

        if (!lexer->input_stream) {
            volt_fmt_logf(VOLT_FMT_LEVEL_ERROR, "Failed to read: {s}", input_file);
            return VOLT_FAILURE;
        }

        volt_lexer_init(lexer, compiler->allocator);
        volt_lexer_lex(lexer);

        volt_parser_init(parser, compiler->allocator, &lexer->tokens, &compiler->error_handler,
                         lexer->input_stream_name);
    }

    return VOLT_SUCCESS;
}

volt_status_code_t volt_parse(volt_compiler_t* compiler) {
    for (size_t i = 0; i < compiler->args.input_count; i++) {
        volt_parser_t* parser = &compiler->parsers[i];
        volt_parser_parse(parser);
    }
    return VOLT_SUCCESS;
}

volt_status_code_t volt_analyze(volt_compiler_t* compiler) {
    // Collect all AST roots and filenames
    volt_ast_node_t** asts =
        (volt_ast_node_t**) compiler->allocator->malloc(sizeof(volt_ast_node_t*) *
                                                        compiler->args.input_count);
    const char** filenames =
        (const char**) compiler->allocator->malloc(sizeof(const char*) *
                                                   compiler->args.input_count);

    for (size_t i = 0; i < compiler->args.input_count; i++) {
        asts[i]       = compiler->parsers[i].root;
        filenames[i]  = compiler->args.input_files[i];
    }

    // Initialize semantic analyzer with all ASTs
    volt_semantic_analyzer_init(&compiler->analyzer, compiler->allocator, asts, filenames,
                                compiler->args.input_count, &compiler->error_handler);

    // Run semantic analysis
    volt_status_code_t result = volt_semantic_analyzer_analyze(&compiler->analyzer);

    // Cleanup temporary arrays
    compiler->allocator->free(asts);
    compiler->allocator->free(filenames);

    return result;
}

volt_status_code_t volt_compile(volt_compiler_t* compiler) {
    (void) compiler;
    return 1;
}

volt_status_code_t volt_link(volt_compiler_t* compiler) {
    (void) compiler;
    return 1;
}
