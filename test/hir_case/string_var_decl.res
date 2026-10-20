Function main
0: MOV q0, s0
1: STR slot(a), q0
2: LOAD_DATA q1, s0_len
3: STR slot(a.len), q1
4: MOV q1, s1
5: STR slot(b), q1
6: LOAD_DATA q2, s1_len
7: STR slot(b.len), q2
8: t3 = INT_CONST 0
9: EXIT t3
