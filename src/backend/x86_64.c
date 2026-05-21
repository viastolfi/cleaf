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
    X86_REG_COUNT,
} x86_reg;

static const char* x86_regs[] = {
    [X86_RBX] = "rbx",
    [X86_R11] = "r11",
    [X86_R12] = "r12",
    [X86_R13] = "r13",
    [X86_R14] = "r14",
    [X86_R15] = "r15",
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

static void x86_emit_add(
    string_builder_t* sb,
    const char* dst,
    int value)
{
  sb_append_fmt(sb, "    add %s, %d\n", dst, value);
}

static void x86_emit_syscall(string_builder_t* sb)
{
  sb_append_fmt(sb, "    syscall\n");
}

const target_t x86_64_target = {
    .setup = x86_setup,
    .regs = x86_regs,
    .reg_count = X86_REG_COUNT,
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
};
