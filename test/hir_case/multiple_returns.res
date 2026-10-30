Function pair
0: t1 = INT_CONST 1
1: t2 = INT_CONST 2
2: ADD t2 t1
3: STR slot($return0_0), t2
4: t3 = INT_CONST 3
5: t4 = INT_CONST 4
6: MUL t4 t3
7: STR slot($return0_1), t4
8: LOAD t5, slot($return0_0)
9: LOAD t6, slot($return0_1)
10: MOV t-1 t5
11: MOV t-2 t6
12: RETURN
Function forward
0: CALL pair
1: MOV t1 t-1
2: MOV t2 t-2
3: MOV t-1 t1
4: MOV t-2 t2
5: RETURN
Function main
0: CALL forward
1: MOV t1 t-1
2: MOV t2 t-2
3: STR slot(a), d1
4: STR slot(b), d2
5: LOAD d3, slot(a)
6: LOAD d4, slot(b)
7: ADD d4 d3
8: EXIT d4
