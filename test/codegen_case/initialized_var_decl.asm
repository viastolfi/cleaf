section .data
section .text
global _start
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov r11, 1
    mov [rbp - 8], r11d
    mov r12, 2
    mov [rbp - 16], r12d
    mov r13, 3
    mov [rbp - 24], r13d
