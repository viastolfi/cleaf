section .text
global _start
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 40
    mov r11, 5
    mov [rbp - 8], r11
    mov r12, [rbp - 8]
    mov r13, r12
    inc r13
    mov [rbp - 8], r13
    mov [rbp - 16], r12
    mov r13, [rbp - 8]
    mov r14, r13
    dec r14
    mov [rbp - 8], r14
    mov [rbp - 24], r13
    mov r14, [rbp - 8]
    inc r14
    mov [rbp - 8], r14
    mov [rbp - 32], r14
    mov r15, [rbp - 8]
    dec r15
    mov [rbp - 8], r15
    mov [rbp - 40], r15
