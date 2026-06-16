section .text
global _start
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    mov rax, 9
    mov rdi, 0
    mov rsi, 3
    mov rdx, 0x01 | 0x02
    mov r10, 0x22
    mov r8, -1
    mov r9, 0
    syscall
    mov [rbp - 8], rax
    mov r11, 0
    add rsp, 8
    pop rbp
    mov rax, 60
    mov rdi, r11
    syscall
