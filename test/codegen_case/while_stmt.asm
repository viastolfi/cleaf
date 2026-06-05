section .text
global _start
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 4
    mov r11, 0
    mov [rbp - 8], r11d
.c0:
    mov r12d, [rbp - 8]
    mov r13, 10
    cmp r12d, r13
    je .c1
    mov r14d, [rbp - 8]
    mov r15, 1
    add r15, r14d
    mov [rbp - 8], r15d
    jmp .c0
.c1:
    mov rbx, [rbp - 8]
    add rsp, 4
    pop rbp
    mov rax, 60
    mov rdi, rbx
    syscall
