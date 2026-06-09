Function main
0: t1 = INT_CONST 42
1: STR slot(a), d1
2: LOAD d2, slot(a)
3: ASM ["mov rax, 60", "mov rdi, %", "syscall"] (d2)
