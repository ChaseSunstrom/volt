#ifndef __VOLT_TOKEN_H__
#define __VOLT_TOKEN_H__

#include <lexer/token_type.h>
#include <util/memory/allocator.h>
#include <util/types/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct volt_token_t volt_token_t;
struct volt_token_t {
    volt_token_type_t type;
    const char*       lexeme;
    size_t            line;
    size_t            column;
    volt_allocator_t* allocator;
};

void               volt_vtoken_deinit(void*);
volt_status_code_t volt_token_deinit(volt_token_t*);
void              volt_token_print(volt_token_t* token);

extern volt_allocator_t volt_token_allocator;

#ifdef __cplusplus
}
#endif

#endif  // __VOLT_TOKEN_H__
