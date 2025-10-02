#include <pch.h>
#include <util/fmt.h>

#if defined(_WIN32)
#    include <io.h>
#    include <windows.h>
#    define isatty _isatty
#    define fileno _fileno
#else
#    include <unistd.h>
#endif

#define C_RESET  "\x1b[0m"
#define C_BOLD   "\x1b[1m"
#define C_RED    "\x1b[31m"
#define C_GREEN  "\x1b[32m"
#define C_YELLOW "\x1b[33m"
#define C_BLUE   "\x1b[34m"
#define C_GRAY   "\x1b[90m"

static inline bool _volt_fmt_enable_color(FILE* stream) {
    if (!isatty(fileno(stream)))
        return false;
    if (getenv("NO_COLOR"))
        return false;  // respect https://no-color.org/
#if defined(_WIN32)
    // Enable ANSI on Windows 10+: turn on VT processing for the console
    DWORD  mode;
    HANDLE h =
        (stream == stdout) ? GetStdHandle(STD_OUTPUT_HANDLE) : GetStdHandle(STD_ERROR_HANDLE);
    if (h == INVALID_HANDLE_VALUE)
        return false;
    if (!GetConsoleMode(h, &mode))
        return false;
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(h, mode))
        return false;
#endif
    return true;
}

static const char* col(bool on, const char* esc) {
    return on ? esc : "";
}

static volt_fmt_level_t* _volt_fmt_get_enabled(void) {
    static bool             inited  = false;
    static volt_fmt_level_t enabled = 0;

    if (!inited) {
        inited = true;
#if defined(VOLT_ENABLE_TRACE)
        enabled |= VOLT_FMT_LEVE_TRACE;
#endif
#if (defined(DEBUG) || defined(VOLT_ENABLE_DEBUG)) && !defined(VOLT_NO_DEBUG_LOGS)
        enabled |= VOLT_FMT_LEVEL_DEBUG;
#endif
#if !defined(VOLT_NO_INFO_LOGS)
        enabled |= VOLT_FMT_LEVEL_INFO;
#endif
        enabled |= VOLT_FMT_LEVEL_WARN;
        enabled |= VOLT_FMT_LEVEL_ERROR;
    }

    return &enabled;
}

// Supported tokens:
//   {s}            -> const char*
//   {i32}            -> int
//   {i64} {u64}    -> int64_t / uint64_t
//   {x}            -> unsigned int, hex (lowercase)
//   {p}            -> const void*
//   {f64}            -> double (prints with %.17g)
//   {f:.N}         -> double with N precision (e.g. {f:.3})
//   {{ / }}        -> literal { / }
// Unknown/missing tokens are printed verbatim like "{...}".
static void _volt_fmt_brace_vprint(FILE* out, const char* fmt, va_list ap) {
    for (const char* p = fmt; *p; ++p) {
        if (*p != '{' && *p != '}') {
            fputc(*p, out);
            continue;
        }

        if (*p == '}' && p[1] == '}') {
            fputc('}', out);
            ++p;
            continue;
        }
        if (*p == '{') {
            if (p[1] == '{') {
                fputc('{', out);
                ++p;
                continue;
            }

            // Parse token between { and }
            const char* t   = p + 1;
            const char* end = strchr(t, '}');
            if (!end) {
                fputc('{', out);
                continue;
            }  // no closing }, print '{' raw

            char   tok[24] = {0};
            size_t len     = (size_t) (end - t);
            if (len >= sizeof(tok))
                len = sizeof(tok) - 1;
            memcpy(tok, t, len);

            // Advance p to the closing '}'
            p = end;

            // Handle tokens
            if (tok[0] == '\0' || strcmp(tok, "s") == 0) {
                const char* s = va_arg(ap, const char*);
                fputs(s ? s : "(null)", out);
                continue;
            }
            if (strcmp(tok, "i32") == 0) {
                int v = va_arg(ap, int);
                fprintf(out, "%d", v);
                continue;
            }
            if (strcmp(tok, "i64") == 0) {
                int64_t v = va_arg(ap, int64_t);
                fprintf(out, "%" PRId64, v);
                continue;
            }
            if (strcmp(tok, "u64") == 0) {
                uint64_t v = va_arg(ap, uint64_t);
                fprintf(out, "%" PRIu64, v);
                continue;
            }
            if (strcmp(tok, "x") == 0) {
                unsigned v = va_arg(ap, unsigned);
                fprintf(out, "%x", v);
                continue;
            }
            if (strcmp(tok, "p") == 0) {
                const void* v = va_arg(ap, const void*);
                fprintf(out, "%p", v);
                continue;
            }
            if (tok[0] == 'f') {
                // default precision
                int32_t prec = -1;  // use %.17g by default
                if (tok[1] == ':' && tok[2] == '.') {
                    prec = 0;
                    for (size_t i = 3; tok[i] && tok[i] >= '0' && tok[i] <= '9'; ++i)
                        prec = prec * 10 + (tok[i] - '0');
                }
                float64_t v = va_arg(ap, double);
                if (prec >= 0) {
                    // build "%.Nf"
                    char fbuf[16];
                    snprintf(fbuf, sizeof(fbuf), "%%.%df", prec);
                    fprintf(out, fbuf, v);
                } else {
                    fprintf(out, "%.17g", v);
                }
                continue;
            }

            // Unknown token: print it back as-is
            fputc('{', out);
            fputs(tok, out);
            fputc('}', out);
        } else {
            // a single '}' that's not part of '}}'
            fputc('}', out);
        }
    }
}

static inline void _volt_fmt_log_v(volt_fmt_level_t lv, const char* fmt, va_list ap) {
    static bool inited = false, color = false;

    static volt_fmt_level_t enabled =
        0;  // Requires all flags to be known before this function runs

    if (!inited) {
        inited  = true;
        enabled = *_volt_fmt_get_enabled();
        color   = _volt_fmt_enable_color(stderr);
    }

    if (!(enabled & lv))
        return;

    const char *tag = "INFO", *pref = "", *suf = col(color, C_RESET);
    switch (lv) {
        case VOLT_FMT_LEVEL_TRACE:
            tag  = "TRACE";
            pref = col(color, C_GRAY);
            break;
        case VOLT_FMT_LEVEL_DEBUG:
            tag  = "DEBUG";
            pref = col(color, C_BLUE);
            break;
        case VOLT_FMT_LEVEL_INFO:
            tag  = "INFO";
            pref = col(color, C_GREEN);
            break;
        case VOLT_FMT_LEVEL_WARN:
            tag  = "WARN";
            pref = col(color, C_YELLOW);
            break;
        case VOLT_FMT_LEVEL_ERROR:
            tag  = "ERROR";
            pref = col(color, C_RED);
            break;
    }

    fprintf(stderr, "%s[%s]%s ", pref, tag, suf);
    _volt_fmt_brace_vprint(stderr, fmt, ap);
    fputc('\n', stderr);
}

void volt_fmt_disable_level(volt_fmt_level_t lv) {
    volt_fmt_level_t* enabled = _volt_fmt_get_enabled();
    *enabled &= ~lv;
}

void volt_fmt_logf(volt_fmt_level_t lv, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    _volt_fmt_log_v(lv, fmt, ap);
    va_end(ap);
}
