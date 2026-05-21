section .text
global _start
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    mov r11, 1
    mov rax, r11
    mov r12, 2
    mov rdi, r12
    call _add
    mov r13, rax
    mov [rbp - 8], r13
    mov r14, [rbp - 8]
    add rsp, 8
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
    mov r11, rdi
    mov [rbp - 16], r11
    mov r13, [rbp - 8]
    mov r14, [rbp - 16]
    add r14, r13
    mov rax, r14
    add rsp, 0
    pop rbp
    ret
