section .text
global _start
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 4
    mov r11, 1
    mov r12, 2
    add r12, r11
    mov [rbp - 8], r12d
