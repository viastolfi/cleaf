section .text
global _start
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 8
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
    mov r14, [rbp - 8]
    mov r15, 2
    imul r15d, 4
    mov ebx, [r14 + r15]
    add rsp, 8
    pop rbp
    mov rax, 60
    mov rdi, rbx
    syscall
