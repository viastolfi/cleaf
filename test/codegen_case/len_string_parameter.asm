section .data
    s1 db 'second', 0
    s1_len dq 6
    s0 db 'first', 0
    s0_len dq 5
section .text
global _start
_print:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    mov rbx, rax
    mov [rbp - 8], rbx
    mov r11, rdi
    mov [rbp - 16], r11
    mov r13, [rbp - 16]
    mov rax, r13
    add rsp, 16
    pop rbp
    ret
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    mov rbx, s0
    mov [rbp - 8], rbx
    mov r11, [rel s0_len]
    mov [rbp - 16], r11
    mov r12, [rbp - 8]
    mov rax, r12
    mov r13, [rbp - 16]
    mov rdi, r13
    call _print
    mov r14, rax
    mov r14, s1
    mov rax, r14
    mov r15, [rel s1_len]
    mov rdi, r15
    call _print
    mov rbx, rax
    mov r11, 0
    add rsp, 16
    pop rbp
    mov rax, 60
    mov rdi, r11
    syscall
