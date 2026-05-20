Function main
0: t1 = INT_CONST 1
1: MOV t-1 t1
2: t2 = INT_CONST 2
3: MOV t-2 t2
4: CALL add
5: MOV t3 t-1
6: STR slot(a), t3
7: LOAD t4, slot(a)
8: EXIT t4
Function add
0: MOV t0 t-1
1: STR slot(a), t0
2: MOV t1 t-2
3: STR slot(b), t1
4: LOAD t3, slot(a)
5: LOAD t4, slot(b)
6: ADD t4 t3
7: MOV t-1 t4
8: RETURN
