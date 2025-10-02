#ifndef __VOLT_DEFINES_H__
#define __VOLT_DEFINES_H__

#define COMPILER_STFU_IM_GONNA_USE_THIS_FILE \
    static int _() {                         \
        return 1;                            \
    }

#ifdef _WIN32
#    define VOLT_WINDOWS
#else
#    define VOLT_UNIX
#endif

#if defined(_MSC_VER) && !defined(VOLT_USE_MSVC)
#    error \
        "Bro, why are you using a shitty compiler? Compile with -DVOLT_USE_MSVC=ON to override. But it may cause issues."
#endif

#endif  // __VOLT_DEFINES_H__
