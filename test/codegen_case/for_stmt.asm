section .text
global _start
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    mov r11, 0
    mov [rbp - 8], r11d
.c0:
    mov r12d, [rbp - 8]
    mov [rbp - 16], r12d
    mov r13d, [rbp - 8]
    inc r13d
    mov [rbp - 8], r13d
    mov r14d, [rbp - 8]
    mov r15, 10
    cmp r14, r15
    jl .c0
    mov rbx, 0
    add rsp, 8
    pop rbp
    mov rax, 60
    mov rdi, rbx
    syscall
