#ifndef TARGET_H
#define TARGET_H

#include "../thirdparty/string_builder.h"

typedef struct {
    void (*setup)(string_builder_t*);

    const char** regs_8;
    int reg_8_count;

    const char** regs_4;
    int reg_4_count;

    const char** reserved_regs;
    int reserved_reg_count;

    void 
      (*emit_mov)
      (string_builder_t*, const char* dst, const char* src);

    void 
      (*emit_ret)
      (string_builder_t*);

    void 
      (*emit_sub_direct)
      (string_builder_t*, const char* dst, int value);

    void 
      (*emit_push)
      (string_builder_t*, const char* dst);

    void 
      (*emit_pop)
      (string_builder_t*, const char* dst);

    void 
      (*func_write)
      (string_builder_t*, const char* name);

    void 
      (*chunk_write)
      (string_builder_t*, const char* name);

    void 
      (*emit_mov_at_stack)
      (string_builder_t*, int place, const char* src);

    void 
      (*emit_add)
      (string_builder_t*, const char* dst, const char* src);

    void 
      (*emit_mov_direct)
      (string_builder_t*, const char* dst, int value);

    void 
      (*emit_syscall)
      (string_builder_t*);

    void 
      (*emit_cmp)
      (string_builder_t*, const char* a, const char* b);

    void 
      (*emit_jmp)
      (string_builder_t*, const char* chunk);

    void 
      (*emit_jmp_equal)
      (string_builder_t*, const char* chunk);

    void 
      (*emit_jmp_not_equal)
      (string_builder_t*, const char* chunk);

    void 
      (*emit_jmp_greater_than)
      (string_builder_t*, const char* chunk);

    void 
      (*emit_jmp_greater_than_equal)
      (string_builder_t*, const char* chunk);

    void 
      (*emit_jmp_lower_than)
      (string_builder_t*, const char* chunk);

    void 
      (*emit_jmp_lower_than_equal)
      (string_builder_t*, const char* chunk);

    void 
      (*emit_call)
      (string_builder_t*, const char* name);

    void 
      (*emit_inc)
      (string_builder_t*, const char* reg);

    void 
      (*emit_dec)
      (string_builder_t*, const char* reg);

    void 
      (*emit_stack_setup)
      (string_builder_t*, int size);

    void 
      (*emit_stack_restore)
      (string_builder_t*, int size);

    void 
      (*emit_process_exit)
      (string_builder_t*, const char* exit_code_reg);

    void 
      (*emit_mov_from_stack)
      (string_builder_t*, const char* dst, int value);

    void 
      (*emit_sub)
      (string_builder_t*, const char* dst, const char* src);

    void 
      (*emit_mul)
      (string_builder_t*, const char* dst, const char* src);

    void
      (*emit_mul_direct)
      (string_builder_t*, const char* dst, int value);

    void 
      (*alloc_memory)
      (string_builder_t*, int size);

    void
      (*emit_mov_offset_pre)
      (string_builder_t*, 
       const char* dst, size_t size, const char* src);

    void
      (*emit_mov_offset_post)
      (string_builder_t*, 
       const char* dst, size_t size, const char* src);

    void
      (*emit_load_elem)
      (string_builder_t*,
       const char* dst, const char* base, const char* index);

    void
      (*dealloc_memory)
      (string_builder_t*,
       const char* src, size_t size);
} target_t;

#endif // TARGET_H
