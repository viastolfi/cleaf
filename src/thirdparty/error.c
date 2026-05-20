#include "error.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

// ANSI escape codes
#define ANSI_RESET   "\033[0m"
#define ANSI_BOLD    "\033[1m"
#define ANSI_DIM     "\033[2m"
#define ANSI_RED     "\033[31m"
#define ANSI_YELLOW  "\033[33m"
#define ANSI_CYAN    "\033[36m"
#define ANSI_MAGENTA "\033[35m"

static int use_color(void) {
    static int cached = -1;
    if (cached == -1) cached = isatty(fileno(stderr));
    return cached;
}

static const char* severity_color(error_severity_t severity) {
    if (!use_color()) return "";
    switch (severity) {
        case ERROR_SEVERITY_ERROR:           return ANSI_BOLD ANSI_RED;
        case ERROR_SEVERITY_WARNING:         return ANSI_BOLD ANSI_YELLOW;
        case ERROR_SEVERITY_NOTE:            return ANSI_BOLD ANSI_CYAN;
        case ERROR_SEVERITY_NOT_IMPLEMENTED: return ANSI_BOLD ANSI_MAGENTA;
        default:                             return "";
    }
}

static const char* severity_label(error_severity_t severity) {
    switch (severity) {
        case ERROR_SEVERITY_NOT_IMPLEMENTED: return "not implemented";
        case ERROR_SEVERITY_ERROR:           return "error";
        case ERROR_SEVERITY_WARNING:         return "warning";
        case ERROR_SEVERITY_NOTE:            return "note";
        default:                             return "error";
    }
}

void error_init(error_context_t* ctx, const char* filename, const char* source, size_t source_len) {
    ctx->filename = filename;
    ctx->source_text = source;
    ctx->source_len = source_len;
}

void error_get_location(const char* source, const char* position, int* line, int* column) {
    *line = 1;
    *column = 1;
    
    for (const char* p = source; p < position && *p; p++) {
        if (*p == '\n') {
            (*line)++;
            *column = 1;
        } else {
            (*column)++;
        }
    }
}

static const char* get_line_start(const char* source, const char* position) {
    const char* line_start = position;
    while (line_start > source && *(line_start - 1) != '\n') {
        line_start--;
    }
    return line_start;
}

static const char* get_line_end(const char* position) {
    const char* line_end = position;
    while (*line_end && *line_end != '\n') {
        line_end++;
    }
    return line_end;
}

static void print_source_line(const char* line_start, const char* line_end, int line_num) {
    if (use_color())
        fprintf(stderr, ANSI_DIM " %4d |" ANSI_RESET " ", line_num);
    else
        fprintf(stderr, " %4d | ", line_num);

    fwrite(line_start, 1, line_end - line_start, stderr);
    fprintf(stderr, "\n");
}

static void print_caret_line(int column, error_severity_t severity) {
    if (use_color())
        fprintf(stderr, ANSI_DIM "      |" ANSI_RESET " ");
    else
        fprintf(stderr, "      | ");

    for (int i = 1; i < column; i++)
        fprintf(stderr, " ");

    if (use_color())
        fprintf(stderr, "%s^" ANSI_RESET "\n", severity_color(severity));
    else
        fprintf(stderr, "^\n");
}

static void print_error_header(const char* filename, int line, int column, error_severity_t severity) {
    const char* color = severity_color(severity);
    const char* reset = use_color() ? ANSI_RESET : "";
    const char* bold  = use_color() ? ANSI_BOLD  : "";
    const char* label = severity_label(severity);

    if (filename && line > 0)
        fprintf(stderr, "%s%s:%d:%d:%s %s%s%s: ",
                bold, filename, line, column, reset, color, label, reset);
    else if (filename)
        fprintf(stderr, "%s%s:%s %s%s%s: ",
                bold, filename, reset, color, label, reset);
    else
        fprintf(stderr, "%s%s%s: ", color, label, reset);
}

void error_report_at_position(error_context_t* ctx, const char* position, error_severity_t severity, const char* fmt, ...) {
    if (!ctx || !ctx->source_text || !position) {
        va_list args;
        va_start(args, fmt);
        print_error_header(NULL, 0, 0, severity);
        vfprintf(stderr, fmt, args);
        fprintf(stderr, "\n");
        va_end(args);
        return;
    }
    
    int line, column;
    error_get_location(ctx->source_text, position, &line, &column);
    
    print_error_header(ctx->filename, line, column, severity);
    
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    
    const char* line_start = get_line_start(ctx->source_text, position);
    const char* line_end = get_line_end(position);
    
    print_source_line(line_start, line_end, line);
    print_caret_line(column, severity);
}

void error_report_at_token(error_context_t* ctx, token_t* token, error_severity_t severity, const char* fmt, ...) {
    if (!ctx || !ctx->source_text || !token || !token->source_pos) {
        va_list args;
        va_start(args, fmt);
        print_error_header(ctx ? ctx->filename : NULL, 0, 0, severity);
        vfprintf(stderr, fmt, args);
        fprintf(stderr, "\n");
        va_end(args);
        return;
    }
    
    int line, column;
    error_get_location(ctx->source_text, token->source_pos, &line, &column);
    
    print_error_header(ctx->filename, line, column, severity);
    
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    
    const char* line_start = get_line_start(ctx->source_text, token->source_pos);
    const char* line_end = get_line_end(token->source_pos);
    
    print_source_line(line_start, line_end, line);
    print_caret_line(column, severity);
}

void error_report_general(error_severity_t severity, const char* fmt, ...) {
    const char* color = severity_color(severity);
    const char* reset = use_color() ? ANSI_RESET : "";
    const char* bold  = use_color() ? ANSI_BOLD  : "";
    const char* label = severity_label(severity);

    fprintf(stderr, "%scleaf%s: %s%s%s: ", bold, reset, color, label, reset);

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}
