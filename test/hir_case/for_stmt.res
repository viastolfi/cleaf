Function main
0: t1 = INT_CONST 0
1: STR slot(a), t1
2: .c0:
3: LOAD t2, slot(a)
4: STR slot(b), t2
5: LOAD t3, slot(a)
6: INC t3
7: STR slot(a), t3
8: LOAD t4, slot(a)
9: t5 = INT_CONST 10
10: CMP t5 t4
11: JL .c0
12: t6 = INT_CONST 0
13: EXIT t6
