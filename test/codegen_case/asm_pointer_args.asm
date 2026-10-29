section .data
section .text
global _start
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    mov rax, 9
    mov rdi, 0
    mov rsi, 100
    mov rdx, 0x01 | 0x02
    mov r10, 0x22
    mov r8, -1
    mov r9, 0
    syscall
    mov rbx, rax
    mov [rbp - 16], rbx
    mov r12, [rbp - 16]
    mov r11, r12
    mov [rbp - 8], r11
    mov r13, [rbp - 8]
    mov r14, [rbp - 16]
    mov r13, r14
    mov [rbp - 8], r13
    mov r15, 0
    add rsp, 16
    pop rbp
    mov rax, 60
    mov rdi, r15
    syscall
