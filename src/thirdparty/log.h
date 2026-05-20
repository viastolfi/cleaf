#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

typedef enum {
    LOG_SILENT  = 0,
    LOG_VERBOSE = 1,
    LOG_DUMP    = 2,
} log_verbosity_t;

void log_set_verbosity(log_verbosity_t level);
log_verbosity_t log_get_verbosity(void);
int log_is_verbose(void);
int log_is_dump(void);

void log_phase(const char* tag, const char* detail_fmt, ...);

void log_section_begin(const char* title);
void log_section_end(void);

#ifdef LOG_LIB_IMPLEMENTATION

#define _L_RESET  "\033[0m"
#define _L_BOLD   "\033[1m"
#define _L_DIM    "\033[2m"
#define _L_CYAN   "\033[36m"
#define _L_YELLOW "\033[33m"

static log_verbosity_t _log_verbosity  = LOG_SILENT;
static int             _log_color_init = -1;

static int _log_use_color(void) {
    if (_log_color_init == -1) _log_color_init = isatty(fileno(stderr));
    return _log_color_init;
}

void log_set_verbosity(log_verbosity_t level) { _log_verbosity = level; }
log_verbosity_t log_get_verbosity(void)       { return _log_verbosity; }
int log_is_verbose(void) { return _log_verbosity >= LOG_VERBOSE; }
int log_is_dump(void)    { return _log_verbosity >= LOG_DUMP; }

void log_phase(const char* tag, const char* detail_fmt, ...) {
    if (!log_is_verbose()) return;

    if (_log_use_color())
        fprintf(stderr, _L_BOLD _L_CYAN "  --> " _L_RESET _L_BOLD "%-16s" _L_RESET, tag);
    else
        fprintf(stderr, "  --> %-16s", tag);

    if (detail_fmt) {
        if (_log_use_color()) fprintf(stderr, _L_DIM);
        va_list args;
        va_start(args, detail_fmt);
        vfprintf(stderr, detail_fmt, args);
        va_end(args);
        if (_log_use_color()) fprintf(stderr, _L_RESET);
    }
    fprintf(stderr, "\n");
}

void log_section_begin(const char* title) {
    if (!log_is_dump()) return;
    if (_log_use_color())
        fprintf(stderr,
            "\n" _L_BOLD _L_YELLOW "  ,-- " _L_RESET _L_BOLD "%s\n" _L_RESET
            _L_DIM _L_YELLOW "  |\n" _L_RESET,
            title);
    else
        fprintf(stderr, "\n  ,-- %s\n  |\n", title);
}

void log_section_end(void) {
    if (!log_is_dump()) return;
    if (_log_use_color())
        fprintf(stderr,
            _L_DIM _L_YELLOW "  |\n"
            "  `----------------------------------\n\n" _L_RESET);
    else
        fprintf(stderr, "  |\n  `----------------------------------\n\n");
}

#undef _L_RESET
#undef _L_BOLD
#undef _L_DIM
#undef _L_CYAN
#undef _L_YELLOW

#endif // LOG_LIB_IMPLEMENTATION
#endif // LOG_H
