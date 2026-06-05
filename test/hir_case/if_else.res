Function main
0: t1 = INT_CONST 0
1: STR slot(a), d1
2: LOAD d2, slot(a)
3: t3 = INT_CONST 0
4: CMP t3 t2
5: JNE .c1
6: t4 = INT_CONST 0
7: EXIT t4
8: JMP .c0
9: .c1:
10: t5 = INT_CONST 1
11: EXIT t5
12: .c0:
13: t6 = INT_CONST 2
14: EXIT t6
