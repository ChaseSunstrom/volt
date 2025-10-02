#ifndef __VOLT_FMT_H__
#define __VOLT_FMT_H__

#include <util/types/array.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum volt_fmt_level_t volt_fmt_level_t;
enum volt_fmt_level_t {
    VOLT_FMT_LEVEL_TRACE = 1,
    VOLT_FMT_LEVEL_DEBUG = 1 << 1,
    VOLT_FMT_LEVEL_INFO  = 1 << 2,
    VOLT_FMT_LEVEL_WARN  = 1 << 3,
    VOLT_FMT_LEVEL_ERROR = 1 << 4
};

void volt_fmt_logf(volt_fmt_level_t, const char*, ...);
void volt_fmt_disable_level(volt_fmt_level_t);

#ifdef __cplusplus
}
#endif

#endif  // __VOLT_FMT_H__
