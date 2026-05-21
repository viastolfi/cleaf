section .text
global _start
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    mov r11, 0
    mov [rbp - 8], r11
.c0:
    mov r12, [rbp - 8]
    mov r13, 10
    cmp r12, r13
    je .c1
    mov r14, [rbp - 8]
    mov r15, 1
    add r15, r14
    mov [rbp - 8], r15
    jmp .c0
.c1:
    mov rbx, [rbp - 8]
    add rsp, 8
    pop rbp
    mov rax, 60
    mov rdi, rbx
    syscall
