Function main
0: ALLOC 8
1: MOV t0 t-1
2: STR slot(a), d0
3: LOAD d1, slot(a)
4: DEALLOC t1, 8
5: MOV t1 t-1
6: STR slot(err), d1
7: LOAD d2, slot(err)
8: EXIT d2
