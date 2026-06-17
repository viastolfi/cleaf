section .text
global _start
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 12
    mov rax, 9
    mov rdi, 0
    mov rsi, 12
    mov rdx, 0x01 | 0x02
    mov r10, 0x22
    mov r8, -1
    mov r9, 0
    syscall
    mov [rbp - 8], rax
    mov rbx, [rbp - 8]
    mov r11, 1
    mov [rbx + 0], r11d
    mov r12, 2
    mov [rbx + 4], r12d
    mov r13, 3
    mov [rbx + 8], r13d
    mov r14, 1
    mov [rbp - 16], r14d
    mov r15, [rbp - 8]
    mov ebx, [rbp - 16]
    imul ebx, 4
    mov r11d, [r15 + rbx]
    add rsp, 12
    pop rbp
    mov rax, 60
    mov rdi, r11
    syscall
