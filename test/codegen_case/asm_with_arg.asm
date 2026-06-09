section .text
global _start
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 4
    mov r11, 42
    mov [rbp - 8], r11d
    mov r12d, [rbp - 8]
    mov rax, 60
    mov rdi, r12
    syscall
