#include "x86_64_definition.h"

typedef enum {
  X86_RAX = 0,
  X86_RDI,
  X86_RSI,
  X86_RDX,
  X86_R8,
  X86_R9,
  X86_R10,
  X86_RESERVED_COUNT,
} x86_reserved_reg;

typedef enum {
  X86_RBX = 0,
  X86_R11,
  X86_R12,
  X86_R13,
  X86_R14,
  X86_R15,
  X86_REG_8_COUNT,
} x86_reg_8;

typedef enum {
  X86_EBX = 0,
  X86_R11D,
  X86_R12D,
  X86_R13D,
  X86_R14D,
  X86_R15D,
  X86_REG_4_COUNT,
} x86_reg_4;

static const char* x86_regs_8[] = {
  [X86_RBX] = "rbx",
  [X86_R11] = "r11",
  [X86_R12] = "r12",
  [X86_R13] = "r13",
  [X86_R14] = "r14",
  [X86_R15] = "r15",
};

static const char* x86_regs_4[] = {
  [X86_EBX]  = "ebx",
  [X86_R11D] = "r11d",
  [X86_R12D] = "r12d",
  [X86_R13D] = "r13d",
  [X86_R14D] = "r14d",
  [X86_R15D] = "r15d",
};

static const char* x86_reserved_regs[] = {
  [X86_RAX] = "rax",
  [X86_RDI] = "rdi",
  [X86_RSI] = "rsi",
  [X86_RDX] = "rdx",
  [X86_R8]  = "r8",
  [X86_R9]  = "r9",
  [X86_R10] = "r10",
};

static void x86_emit_mov(
    string_builder_t* sb, 
    const char* dst, 
    const char* src) {
  sb_append_fmt(sb, "    mov %s, %s\n", dst, src);
}

static void x86_emit_mov_direct(
    string_builder_t* sb,
    const char* dst,
    int value)
{
  sb_append_fmt(sb, "    mov %s, %d\n", dst, value);
}

static void x86_emit_ret(string_builder_t* sb) {
  sb_append_fmt(sb, "    ret\n");
}

static void x86_emit_sub(
    string_builder_t* sb,
    const char* dst,
    const char* src)
{
  sb_append_fmt(sb, "    sub %s, %s\n", dst, src);
}

static void x86_emit_mul(
    string_builder_t* sb,
    const char* dst,
    const char* src)
{
  sb_append_fmt(sb, "    imul %s, %s\n", dst, src);
}

static void x86_emit_sub_direct(
    string_builder_t* sb, 
    const char* dst, 
    int value) {
  sb_append_fmt(sb, "    sub %s, %d\n", dst, value);
}

static void x86_emit_push(
    string_builder_t* sb,
    const char* dst)
{
  sb_append_fmt(sb, "    push %s\n", dst);
}

static void x86_emit_pop(
    string_builder_t* sb,
    const char* dst)
{
  sb_append_fmt(sb, "    pop %s\n", dst);
}

static void x86_setup(
    string_builder_t* sb)
{
  sb_append_fmt(sb, "section .text\nglobal _start\n");
}

static void x86_func_write(
    string_builder_t* sb,
    const char* name)
{
  sb_append_fmt(sb, "_%s:\n", name);
}

static void x86_chunk_write(
    string_builder_t* sb,
    const char* name) 
{
  sb_append_fmt(sb, "%s:\n", name);
}

static void x86_emit_mov_at_stack(
    string_builder_t* sb,
    int place,
    const char* src) 
{
  sb_append_fmt(sb, "    mov [rbp - %d], %s\n", place, src);
}

static void x86_emit_mov_from_stack(
    string_builder_t* sb,
    const char* dst,
    int place)
{
  sb_append_fmt(sb, "    mov %s, [rbp - %d]\n", dst, place);
}

static void x86_emit_add(
    string_builder_t* sb,
    const char* dst,
    const char* src)
{
  sb_append_fmt(sb, "    add %s, %s\n", dst, src);
}

static void x86_emit_jmp(
    string_builder_t* sb,
    const char* chunk)
{
  sb_append_fmt(sb, "    jmp %s\n", chunk);
}

static void x86_emit_je(
    string_builder_t* sb,
    const char* chunk)
{
  sb_append_fmt(sb, "    je %s\n", chunk);
}

static void x86_emit_jne(
    string_builder_t* sb,
    const char* chunk)
{
  sb_append_fmt(sb, "    jne %s\n", chunk);
}

static void x86_emit_jl(
    string_builder_t* sb,
    const char* chunk)
{
  sb_append_fmt(sb, "    jl %s\n", chunk);
}

static void x86_emit_jle(
    string_builder_t* sb,
    const char* chunk)
{
  sb_append_fmt(sb, "    jle %s\n", chunk);
}

static void x86_emit_jg(
    string_builder_t* sb,
    const char* chunk)
{
  sb_append_fmt(sb, "    jg %s\n", chunk);
}

static void x86_emit_jge(
    string_builder_t* sb,
    const char* chunk)
{
  sb_append_fmt(sb, "    jge %s\n", chunk);
}

static void x86_emit_syscall(string_builder_t* sb)
{
  sb_append_fmt(sb, "    syscall\n");
}

static void x86_emit_cmp(
    string_builder_t* sb,
    const char* a,
    const char* b)
{
  sb_append_fmt(sb, "    cmp %s, %s\n", a, b);
}

static void x86_emit_call(
    string_builder_t* sb,
    const char* name)
{
  sb_append_fmt(sb, "    call _%s\n", name);
}

static void x86_emit_inc(string_builder_t* sb, const char* reg)
{
  sb_append_fmt(sb, "    inc %s\n", reg);
}

static void x86_emit_dec(string_builder_t* sb, const char* reg)
{
  sb_append_fmt(sb, "    dec %s\n", reg);
}

static void x86_emit_stack_setup(string_builder_t* sb, int size)
{
  sb_append_fmt(sb, "    push rbp\n");
  sb_append_fmt(sb, "    mov rbp, rsp\n");
  sb_append_fmt(sb, "    sub rsp, %d\n", size);
}

static void x86_emit_stack_restore(string_builder_t* sb, int size)
{
  sb_append_fmt(sb, "    add rsp, %d\n", size);
  sb_append_fmt(sb, "    pop rbp\n");
}

static void x86_emit_process_exit(
    string_builder_t* sb, const char* exit_code_reg)
{
  sb_append_fmt(sb, "    mov rax, 60\n");
  sb_append_fmt(sb, "    mov rdi, %s\n", exit_code_reg);
  sb_append_fmt(sb, "    syscall\n");
}

static void x86_alloc_memory(string_builder_t* sb, int size) 
{
  sb_append_fmt(sb, "    mov rax, 9\n");
  sb_append_fmt(sb, "    mov rdi, 0\n");
  sb_append_fmt(sb, "    mov rsi, %d\n", size);
  sb_append_fmt(sb, "    mov rdx, 0x01 | 0x02\n");
  sb_append_fmt(sb, "    mov r10, 0x22\n");
  sb_append_fmt(sb, "    mov r8, -1\n");
  sb_append_fmt(sb, "    mov r9, 0\n");
  sb_append_fmt(sb, "    syscall\n");
}

static void x86_emit_mov_offset_pre(string_builder_t* sb,
    const char* dst, size_t size, const char* src)
{
  sb_append_fmt(sb, "    mov [%s + %zu], %s\n", dst, size, src);
}

static void x86_emit_mov_offset_post(string_builder_t* sb,
    const char* dst, size_t size, const char* src)
{
  sb_append_fmt(sb, "    mov %s, [%s + %zu]\n", dst, src, size);
}

const target_t x86_64_target = {
  .setup = x86_setup,
  .regs_8 = x86_regs_8,
  .reg_8_count = X86_REG_8_COUNT,
  .regs_4 = x86_regs_4,
  .reg_4_count = X86_REG_4_COUNT,
  .reserved_regs = x86_reserved_regs,
  .reserved_reg_count = X86_RESERVED_COUNT,
  .emit_mov = x86_emit_mov,
  .emit_ret = x86_emit_ret,
  .emit_sub_direct = x86_emit_sub_direct,
  .emit_push = x86_emit_push,
  .emit_pop = x86_emit_pop,
  .func_write = x86_func_write,
  .chunk_write = x86_chunk_write,
  .emit_mov_at_stack = x86_emit_mov_at_stack,
  .emit_add = x86_emit_add,
  .emit_mov_direct = x86_emit_mov_direct,
  .emit_syscall = x86_emit_syscall,
  .emit_cmp = x86_emit_cmp,
  .emit_jmp = x86_emit_jmp,
  .emit_jmp_equal = x86_emit_je,
  .emit_jmp_not_equal = x86_emit_jne,
  .emit_jmp_greater_than = x86_emit_jg,
  .emit_jmp_greater_than_equal = x86_emit_jge,
  .emit_jmp_lower_than = x86_emit_jl,
  .emit_jmp_lower_than_equal = x86_emit_jle,
  .emit_call = x86_emit_call,
  .emit_inc = x86_emit_inc,
  .emit_dec = x86_emit_dec,
  .emit_stack_setup = x86_emit_stack_setup,
  .emit_stack_restore = x86_emit_stack_restore,
  .emit_process_exit = x86_emit_process_exit,
  .emit_mov_from_stack = x86_emit_mov_from_stack,
  .emit_sub = x86_emit_sub,
  .emit_mul = x86_emit_mul,
  .alloc_memory = x86_alloc_memory,
  .emit_mov_offset_pre = x86_emit_mov_offset_pre,
  .emit_mov_offset_post = x86_emit_mov_offset_post,
};
