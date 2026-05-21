#ifndef TARGET_H
#define TARGET_H

#include "../thirdparty/string_builder.h"

typedef struct {
    void (*setup)(string_builder_t*);

    const char** regs;
    int reg_count;

    const char** reserved_regs;
    int reserved_reg_count;

    void (*emit_mov)(string_builder_t*, const char* dst, const char* src);
    void (*emit_ret)(string_builder_t*);
    void (*emit_sub_direct)(string_builder_t*, const char* dst, int value);
    void (*emit_push)(string_builder_t*, const char* dst);
    void (*emit_pop)(string_builder_t*, const char* dst);

    void (*func_write)(string_builder_t*, const char* name);
    void (*chunk_write)(string_builder_t*, const char* name);
    void (*emit_mov_at_stack)(string_builder_t*, int place, const char* src);
    void (*emit_add)(string_builder_t*, const char* dst, int value);
    void (*emit_mov_direct)(string_builder_t*, const char* dst, int value);
    void (*emit_syscall)(string_builder_t*);
} target_t;

#endif // TARGET_H
