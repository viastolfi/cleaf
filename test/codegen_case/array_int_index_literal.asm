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
    mov r13, [rbp - 8]
    mov r14, 0
    imul r14d, 4
    mov r15d, [r13 + r14]
    add rsp, 8
    pop rbp
    mov rax, 60
    mov rdi, r15
    syscall
