Function main
0: ALLOC 8
1: STR slot(a), t-1
2: LOAD t0, slot(a)
3: t1 = INT_CONST 1
4: MOV [t0 + 0], t1
5: t2 = INT_CONST 2
6: MOV [t0 + 4], t2
7: t3 = INT_CONST 0
8: EXIT t3
