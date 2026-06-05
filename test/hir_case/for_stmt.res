Function main
0: t1 = INT_CONST 0
1: STR slot(a), d1
2: .c0:
3: LOAD d2, slot(a)
4: STR slot(b), d2
5: LOAD d3, slot(a)
6: INC d3
7: STR slot(a), d3
8: LOAD d4, slot(a)
9: t5 = INT_CONST 10
10: CMP t5 d4
11: JL .c0
12: t6 = INT_CONST 0
13: EXIT t6
