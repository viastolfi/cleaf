section .text
global _start
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 4
    mov r11, 0
    mov [rbp - 8], r11d
    mov r12d, [rbp - 8]
    mov r13, 0
    cmp r12, r13
    jne .c0
    mov r14d, [rbp - 8]
    add rsp, 4
    pop rbp
    mov rax, 60
    mov rdi, r14
    syscall
.c0:
    mov r15d, [rbp - 8]
    mov rbx, 0
    cmp r15, rbx
    je .c1
    mov r11d, [rbp - 8]
    add rsp, 4
    pop rbp
    mov rax, 60
    mov rdi, r11
    syscall
.c1:
    mov r12d, [rbp - 8]
    mov r13, 12
    cmp r12, r13
    jge .c2
    mov r14d, [rbp - 8]
    add rsp, 4
    pop rbp
    mov rax, 60
    mov rdi, r14
    syscall
.c2:
    mov r15d, [rbp - 8]
    mov rbx, 13
    cmp r15, rbx
    jg .c3
    mov r11d, [rbp - 8]
    add rsp, 4
    pop rbp
    mov rax, 60
    mov rdi, r11
    syscall
.c3:
    mov r12d, [rbp - 8]
    mov r13, 14
    cmp r12, r13
    jle .c4
    mov r14d, [rbp - 8]
    add rsp, 4
    pop rbp
    mov rax, 60
    mov rdi, r14
    syscall
.c4:
    mov r15d, [rbp - 8]
    mov rbx, 14
    cmp r15, rbx
    jl .c5
    mov r11d, [rbp - 8]
    add rsp, 4
    pop rbp
    mov rax, 60
    mov rdi, r11
    syscall
.c5:
    mov r12d, [rbp - 8]
    add rsp, 4
    pop rbp
    mov rax, 60
    mov rdi, r12
    syscall
