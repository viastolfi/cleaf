section .text
global _start
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 4
    mov r11, 1
    mov rax, r11
    mov r12, 2
    mov rdi, r12
    call _add
    mov r13, rax
    mov [rbp - 8], r13d
    mov r14d, [rbp - 8]
    add rsp, 4
    pop rbp
    mov rax, 60
    mov rdi, r14
    syscall
_add:
    push rbp
    mov rbp, rsp
    sub rsp, 0
    mov rbx, rax
    mov [rbp - 8], rbx
    mov r11d, rdi
    mov [rbp - 16], r11d
    mov r13d, [rbp - 8]
    mov r14d, [rbp - 16]
    add r14d, r13d
    mov rax, r14d
    add rsp, 0
    pop rbp
    ret
