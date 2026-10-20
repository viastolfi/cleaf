section .data
    s1 db 'second', 0
    s1_len dq 6
    s0 db 'first', 0
    s0_len dq 5
section .text
global _start
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 32
    mov rbx, s0
    mov [rbp - 8], rbx
    mov r11, [rel s0_len]
    mov [rbp - 16], r11
    mov r11, s1
    mov [rbp - 24], r11
    mov r12, [rel s1_len]
    mov [rbp - 32], r12
    mov r13, 0
    add rsp, 32
    pop rbp
    mov rax, 60
    mov rdi, r13
    syscall
