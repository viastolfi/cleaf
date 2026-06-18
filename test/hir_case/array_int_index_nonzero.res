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
9: LOAD q4, slot(a)
10: t5 = INT_CONST 2
11: MUL d5, 4
12: MOV d6, [q4 + q5]
13: EXIT d6
