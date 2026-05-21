section .text
global _start
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov r11, 12
    mov [rbp - 8], r11
    mov r12, [rbp - 8]
    mov [rbp - 16], r12
    mov r13, [rbp - 16]
    mov r14, [rbp - 8]
    add r14, r13
    mov [rbp - 24], r14
