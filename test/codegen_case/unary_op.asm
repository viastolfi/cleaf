section .text
global _start
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 20
    mov r11, 5
    mov [rbp - 8], r11d
    mov r12d, [rbp - 8]
    mov r13d, r12d
    inc r13d
    mov [rbp - 8], r13d
    mov [rbp - 16], r12d
    mov r13d, [rbp - 8]
    mov r14d, r13d
    dec r14d
    mov [rbp - 8], r14d
    mov [rbp - 24], r13d
    mov r14d, [rbp - 8]
    inc r14d
    mov [rbp - 8], r14d
    mov [rbp - 32], r14d
    mov r15d, [rbp - 8]
    dec r15d
    mov [rbp - 8], r15d
    mov [rbp - 40], r15d
