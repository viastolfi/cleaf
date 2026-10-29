section .data
section .text
global _start
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    mov r11, 12
    mov [rbp - 8], r11d
    mov r12d, [rbp - 8]
    add r12, 1
    mov [rbp - 8], r12d
    mov r13d, [rbp - 8]
    add rsp, 8
    pop rbp
    mov rax, 60
    mov rdi, r13
    syscall
