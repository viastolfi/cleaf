#include "error.h"
#include <string.h>
#include <stdlib.h>

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
    fprintf(stderr, " %4d | ", line_num);
    
    fwrite(line_start, 1, line_end - line_start, stderr);
    fprintf(stderr, "\n");
}

static void print_caret_line(int column) {
    fprintf(stderr, "      | ");
    
    for (int i = 1; i < column; i++) {
        fprintf(stderr, " ");
    }
    
    fprintf(stderr, "^\n");
}

static void print_error_header(const char* filename, int line, int column, error_severity_t severity) {
    const char* severity_str;
    
    switch (severity) {
        case ERROR_SEVERITY_ERROR:
            severity_str = "error";
            break;
        case ERROR_SEVERITY_WARNING:
            severity_str = "warning";
            break;
        case ERROR_SEVERITY_NOTE:
            severity_str = "note";
            break;
        default:
            severity_str = "error";
            break;
    }
    
    if (filename && line > 0) {
        fprintf(stderr, "%s:%d:%d: %s: ", filename, line, column, severity_str);
    } else {
        fprintf(stderr, "%s: ", severity_str);
    }
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
    print_caret_line(column);
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
    print_caret_line(column);
}

void error_report_general(error_severity_t severity, const char* fmt, ...) {
    print_error_header(NULL, 0, 0, severity);
    
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}
