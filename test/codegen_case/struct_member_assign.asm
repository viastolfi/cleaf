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
    mov [rbp - 8], rax
    mov rbx, [rbp - 8]
    mov r11, 1
    mov [rbx + 0], r11d
    mov r12, 2
    mov [rbx + 4], r12d
    mov r13, 42
    mov r14, [rbp - 8]
    mov [r14 + 4], r13
    mov r15, [rbp - 8]
    mov ebx, [r15 + 4]
    add rsp, 8
    pop rbp
    mov rax, 60
    mov rdi, rbx
    syscall
