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
    inc r12d
    mov [rbp - 8], r12d
    mov [rbp - 16], r13d
    mov r14d, [rbp - 8]
    mov r15d, r14d
    dec r14d
    mov [rbp - 8], r14d
    mov [rbp - 24], r15d
    mov ebx, [rbp - 8]
    inc ebx
    mov [rbp - 8], ebx
    mov [rbp - 32], ebx
    mov r11d, [rbp - 8]
    dec r11d
    mov [rbp - 8], r11d
    mov [rbp - 40], r11d
