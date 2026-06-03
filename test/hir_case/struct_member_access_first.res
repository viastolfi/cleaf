Function main
0: ALLOC 16
1: STR slot(a), t-1
2: LOAD t0, slot(a)
3: t1 = INT_CONST 3
4: MOV [t0 + 0], t1
5: t2 = INT_CONST 2
6: MOV [t0 + 8], t2
7: LOAD t3, slot(a)
8: MOV t4, [t3 + 0]
9: EXIT t4
