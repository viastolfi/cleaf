Function main
0: ALLOC 8
1: STR slot(a), d-1
2: LOAD q0, slot(a)
3: t1 = INT_CONST 1
4: MOV [q0 + 0], d1
5: t2 = INT_CONST 2
6: MOV [q0 + 4], d2
7: t3 = INT_CONST 67
8: LOAD q4, slot(a)
9: t5 = INT_CONST 0
10: MUL d5, 4
11: MOV [q4 + q5], d3
12: LOAD q6, slot(a)
13: t7 = INT_CONST 0
14: MUL d7, 4
15: MOV d8, [q6 + q7]
16: EXIT d8
