Function main
0: ALLOC 16
1: STR slot(a), t-1
2: LOAD t0, slot(a)
3: t1 = INT_CONST 2
4: MOV [t0 + 8], t1
5: t2 = INT_CONST 1
6: MOV [t0 + 0], t2
7: t3 = INT_CONST 0
8: EXIT t3
