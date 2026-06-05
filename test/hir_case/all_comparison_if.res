Function main
0: t1 = INT_CONST 0
1: STR slot(a), d1
2: LOAD d2, slot(a)
3: t3 = INT_CONST 0
4: CMP t3 t2
5: JNE .c0
6: LOAD d4, slot(a)
7: EXIT d4
8: .c0:
9: LOAD d5, slot(a)
10: t6 = INT_CONST 0
11: CMP t6 t5
12: JE .c1
13: LOAD d7, slot(a)
14: EXIT d7
15: .c1:
16: LOAD d8, slot(a)
17: t9 = INT_CONST 12
18: CMP t9 t8
19: JGE .c2
20: LOAD d10, slot(a)
21: EXIT d10
22: .c2:
23: LOAD d11, slot(a)
24: t12 = INT_CONST 13
25: CMP t12 t11
26: JG .c3
27: LOAD d13, slot(a)
28: EXIT d13
29: .c3:
30: LOAD d14, slot(a)
31: t15 = INT_CONST 14
32: CMP t15 t14
33: JLE .c4
34: LOAD d16, slot(a)
35: EXIT d16
36: .c4:
37: LOAD d17, slot(a)
38: t18 = INT_CONST 14
39: CMP t18 t17
40: JL .c5
41: LOAD d19, slot(a)
42: EXIT d19
43: .c5:
44: LOAD d20, slot(a)
45: EXIT d20
