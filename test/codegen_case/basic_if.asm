section .text
global _start
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 4
    mov r11, 0
    mov [rbp - 8], r11d
    mov r12d, [rbp - 8]
    mov r13, 0
    cmp r12d, r13
    jne .c0
    mov r14, 0
    add rsp, 4
    pop rbp
    mov rax, 60
    mov rdi, r14
    syscall
.c0:
    mov r15, 1
    add rsp, 4
    pop rbp
    mov rax, 60
    mov rdi, r15
    syscall
