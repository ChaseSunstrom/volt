#ifndef __VOLT_ARRAY_H__
#define __VOLT_ARRAY_H__

#include <util/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct volt_array_t volt_array_t;
struct volt_array_t {
    void**  array;
    int64_t length;  // will get padded anyway
};

#ifdef __cplusplus
}
#endif

#endif  // __VOLT_ARRAY_H__
