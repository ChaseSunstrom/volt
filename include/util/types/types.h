#ifndef __VOLT_TYPES_H__
#define __VOLT_TYPES_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef float  float32_t;
typedef double float64_t;

typedef enum volt_status_code_t volt_status_code_t;
enum volt_status_code_t { VOLT_SUCCESS = 1, VOLT_FAILURE = 0 };

#ifdef __cplusplus
}
#endif

#endif  // __VOLT_TYPES_H__
