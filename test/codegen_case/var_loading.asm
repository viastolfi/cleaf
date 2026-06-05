section .text
global _start
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 12
    mov r11, 12
    mov [rbp - 8], r11d
    mov r12d, [rbp - 8]
    mov [rbp - 16], r12d
    mov r13d, [rbp - 16]
    mov r14d, [rbp - 8]
    add r14d, r13d
    mov [rbp - 24], r14d
