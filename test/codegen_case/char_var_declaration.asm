section .text
global _start
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 2
    mov r11, 97
    mov [rbp - 8], r11
    mov r12, 12
    mov [rbp - 16], r12
