#ifndef __VOLT_LEXER_H__
#define __VOLT_LEXER_H__

#include <util/types/types.h>
#include <util/types/vector.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct volt_lexer_t volt_lexer_t;
struct volt_lexer_t {
    const char*    input_stream;
    volt_vector_t* tokens;
};

#ifdef __cplusplus
}
#endif

#endif  // __VOLT_LEXER_H__
