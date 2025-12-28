#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>
#include <stdarg.h>
#include "../frontend/lexer.h"

typedef enum {
    ERROR_SEVERITY_ERROR,
    ERROR_SEVERITY_WARNING,
    ERROR_SEVERITY_NOTE
} error_severity_t;

typedef struct {
    const char* filename;
    const char* source_text;
    size_t source_len;
} error_context_t;

void error_init(error_context_t* ctx, const char* filename, const char* source, size_t source_len);
void error_report_at_token(error_context_t* ctx, token_t* token, error_severity_t severity, const char* fmt, ...);
void error_report_at_position(error_context_t* ctx, const char* position, error_severity_t severity, const char* fmt, ...);
void error_report_general(error_severity_t severity, const char* fmt, ...);
void error_get_location(const char* source, const char* position, int* line, int* column);

#endif // ERROR_H
