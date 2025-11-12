#ifndef __VOLT_LEXER_H__
#define __VOLT_LEXER_H__

#include <util/types/types.h>
#include <util/types/vector.h>
#include <volt/error.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct volt_lexer_t volt_lexer_t;
struct volt_lexer_t {
    const char*           input_stream;
    const char*           input_stream_name;
    size_t                current_position;
    size_t                current_line;
    size_t                current_column;
    size_t                input_stream_length;
    volt_vector_t         tokens;
    volt_error_handler_t* error_handler;
    volt_allocator_t*     allocator;
};

volt_status_code_t volt_lexer_init(volt_lexer_t*, volt_allocator_t*);
volt_status_code_t volt_lexer_deinit(volt_lexer_t*);
volt_status_code_t volt_lexer_lex(volt_lexer_t*);

#ifdef __cplusplus
}
#endif

#endif  // __VOLT_LEXER_H__
