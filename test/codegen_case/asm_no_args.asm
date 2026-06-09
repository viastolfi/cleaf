section .text
global _start
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 0
    mov rax, 60
    mov rdi, 0
    syscall
