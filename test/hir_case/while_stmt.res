Function main
0: t1 = INT_CONST 0
1: STR slot(a), d1
2: .c0:
3: LOAD d2, slot(a)
4: t3 = INT_CONST 10
5: CMP t3 d2
6: JE .c1
7: LOAD d4, slot(a)
8: t5 = INT_CONST 1
9: ADD t5 d4
10: STR slot(a), d5
11: JMP .c0
12: .c1:
13: LOAD d6, slot(a)
14: EXIT d6
