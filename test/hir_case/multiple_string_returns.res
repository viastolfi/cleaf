Function pair
0: t1 = INT_CONST 7
1: STR slot($return0_0), t1
2: MOV q1, s0
3: LOAD_DATA q2, s0_len
4: STR slot($return0_1), q1
5: STR slot($return0_1.len), q2
6: LOAD t3, slot($return0_0)
7: LOAD q4, slot($return0_1)
8: LOAD q5, slot($return0_1.len)
9: MOV t-1 t3
10: MOV t-2 t4
11: MOV t-3 t5
12: RETURN
Function forward
0: CALL pair
1: MOV t1 t-1
2: MOV q2 q-3
3: MOV t3 t-2
4: MOV t-1 t1
5: MOV t-2 t3
6: MOV t-3 t2
7: RETURN
Function main
0: CALL forward
1: MOV t1 t-1
2: MOV q2 q-3
3: MOV t3 t-2
4: STR slot(a), d1
5: STR slot(text), q3
6: STR slot(text.len), q2
7: LOAD d4, slot(a)
8: LOAD q5, slot(text.len)
9: ADD d5 d4
10: EXIT d5
