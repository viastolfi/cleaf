Function main
0: ALLOC 3
1: STR slot(a), b-1
2: LOAD q0, slot(a)
3: t1 = INT_CONST 1
4: MOV [q0 + 0], b1
5: t2 = INT_CONST 2
6: MOV [q0 + 1], b2
7: t3 = INT_CONST 3
8: MOV [q0 + 2], b3
9: LOAD q4, slot(a)
10: t5 = INT_CONST 1
11: MUL b5, 1
12: MOV b6, [q4 + q5]
13: EXIT b6
