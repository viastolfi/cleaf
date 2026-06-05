Function main
0: ALLOC 8
1: STR slot(a), q-1
2: LOAD q0, slot(a)
3: t1 = INT_CONST 3
4: MOV [q0 + 0], d1
5: t2 = INT_CONST 2
6: MOV [q0 + 4], d2
7: LOAD q3, slot(a)
8: MOV d4, [q3 + 0]
9: EXIT d4
