section .data
section .text
global _start
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    mov rax, 9
    mov rdi, 0
    mov rsi, 8
    mov rdx, 0x01 | 0x02
    mov r10, 0x22
    mov r8, -1
    mov r9, 0
    syscall
    mov rbx, rax
    mov [rbp - 8], ebx
    mov r11d, [rbp - 8]
    mov rax, 11
    mov rdi, r11
    mov rsi, 8
    syscall
    mov r11, rax
    mov [rbp - 16], r11d
    mov r12d, [rbp - 16]
    add rsp, 8
    pop rbp
    mov rax, 60
    mov rdi, r12
    syscall
