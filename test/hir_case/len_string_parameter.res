Function print
0: MOV t0 t-1
1: STR slot(value), q0
2: MOV q1 q-2
3: STR slot(value.len), q1
4: LOAD q3, slot(value.len)
5: MOV t-1 t3
6: RETURN
Function main
0: MOV q0, s0
1: STR slot(first), q0
2: LOAD_DATA q1, s0_len
3: STR slot(first.len), q1
4: LOAD q2, slot(first)
5: MOV t-1 t2
6: LOAD q3, slot(first.len)
7: MOV q-2 q3
8: CALL print
9: MOV t4 t-1
10: MOV q4, s1
11: MOV t-1 t4
12: LOAD_DATA q5, s1_len
13: MOV q-2 q5
14: CALL print
15: MOV t6 t-1
16: t7 = INT_CONST 0
17: EXIT t7
