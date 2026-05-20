Function main
0: t1 = INT_CONST 0
1: STR slot(a), t1
2: .c0:
3: LOAD t2, slot(a)
4: t3 = INT_CONST 10
5: CMP t3 t2
6: JE .c1
7: LOAD t4, slot(a)
8: t5 = INT_CONST 1
9: ADD t5 t4
10: STR slot(a), t5
11: JMP .c0
12: .c1:
13: LOAD t6, slot(a)
14: EXIT t6
