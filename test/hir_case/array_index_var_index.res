Function main
0: ALLOC 12
1: STR slot(a), d-1
2: LOAD q0, slot(a)
3: t1 = INT_CONST 1
4: MOV [q0 + 0], d1
5: t2 = INT_CONST 2
6: MOV [q0 + 4], d2
7: t3 = INT_CONST 3
8: MOV [q0 + 8], d3
9: t4 = INT_CONST 1
10: STR slot(i), d4
11: LOAD q5, slot(a)
12: LOAD d6, slot(i)
13: MUL d6, 4
14: MOV d7, [q5 + q6]
15: EXIT d7
