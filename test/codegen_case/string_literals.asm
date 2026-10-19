section .data
    s1 db 'second', 0
    s1_len db 6
    s0 db 'first', 0
    s0_len db 5
section .text
global _start
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    mov rbx, s0
    mov [rbp - 8], rbx
    mov rbx, s1
    mov [rbp - 16], rbx
    mov r11, 0
    add rsp, 16
    pop rbp
    mov rax, 60
    mov rdi, r11
    syscall
