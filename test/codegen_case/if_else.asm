section .text
global _start
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    mov r11, 0
    mov [rbp - 8], r11
    mov r12, [rbp - 8]
    mov r13, 0
    cmp r12, r13
    jne .c1
    mov r14, 0
    add rsp, 8
    pop rbp
    mov rax, 60
    mov rdi, r14
    syscall
    jmp .c0
.c1:
    mov r15, 1
    add rsp, 8
    pop rbp
    mov rax, 60
    mov rdi, r15
    syscall
.c0:
    mov rbx, 2
    add rsp, 8
    pop rbp
    mov rax, 60
    mov rdi, rbx
    syscall
