section .text
global _start
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    mov r11, 0
    mov [rbp - 8], r11
.c0:
    mov r12, [rbp - 8]
    mov [rbp - 16], r12
    mov r13, [rbp - 8]
    inc r13
    mov [rbp - 8], r13
    mov r14, [rbp - 8]
    mov r15, 10
    cmp r14, r15
    jl .c0
    mov rbx, 0
    add rsp, 16
    pop rbp
    mov rax, 60
    mov rdi, rbx
    syscall
