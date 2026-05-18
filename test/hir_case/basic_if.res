Function main
0: t1 = INT_CONST 0
1: STR slot(a), t1
2: LOAD t2, slot(a)
3: t3 = INT_CONST 0
4: CMP t2, t3
5: JNE .c0
6: t5 = INT_CONST 0
7: EXIT t5
8: .c0:
9: t6 = INT_CONST 1
10: EXIT t6
