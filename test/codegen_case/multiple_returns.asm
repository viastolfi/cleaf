section .data
section .text
global _start
_pair:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    mov r11, 7
    mov [rbp - 8], r11
    mov r12, 9
    mov [rbp - 16], r12
    mov r13, [rbp - 8]
    mov r14, [rbp - 16]
    mov rax, r13
    mov rdi, r14
    add rsp, 16
    pop rbp
    ret
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    call _pair
    mov r11, rax
    mov r12, rdi
    mov [rbp - 8], r11d
    mov [rbp - 16], r12d
    mov r13d, [rbp - 8]
    mov r14d, [rbp - 16]
    add r14d, r13d
    add rsp, 16
    pop rbp
    mov rax, 60
    mov rdi, r14
    syscall
