section .text
global _start
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 12
    mov r11, 5
    mov [rbp - 8], r11d
    mov r12, 3
    mov [rbp - 16], r12d
    mov r13d, [rbp - 8]
    mov r14d, [rbp - 16]
    add r14d, r13d
    mov [rbp - 24], r14d
    mov r15d, [rbp - 24]
    add rsp, 12
    pop rbp
    mov rax, 60
    mov rdi, r15
    syscall
