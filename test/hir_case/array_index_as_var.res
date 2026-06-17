Function main
0: ALLOC 8
1: STR slot(a), d-1
2: LOAD q0, slot(a)
3: t1 = INT_CONST 1
4: MOV [q0 + 0], d1
5: t2 = INT_CONST 2
6: MOV [q0 + 4], d2
7: LOAD q3, slot(a)
8: t4 = INT_CONST 0
9: MUL d4, 4
10: MOV d5, [q3 + q4]
11: STR slot(b), d5
12: LOAD d6, slot(b)
13: EXIT d6
