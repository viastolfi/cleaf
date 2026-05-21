section .text
global _start
_start:
    push rbp
    mov rbp, rsp
    sub rsp, 0
    mov r11, 0
    add rsp, 0
    pop rbp
    mov rax, 60
    mov rdi, r11
    syscall
_foo:
    push rbp
    mov rbp, rsp
    sub rsp, 0
    mov r11, 0
    mov rax, r11
    add rsp, 0
    pop rbp
    ret
