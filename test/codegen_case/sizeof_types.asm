section .data
section .text
global _start
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov rbx, 4
    mov [rbp - 8], ebx
    mov rbx, 4
    mov [rbp - 16], ebx
    mov rbx, 5
    mov [rbp - 24], ebx
    mov r11d, [rbp - 8]
    mov r12d, [rbp - 16]
    add r12d, r11d
    mov r13d, [rbp - 24]
    add r13d, r12d
    add rsp, 24
    pop rbp
    mov rax, 60
    mov rdi, r13
    syscall
