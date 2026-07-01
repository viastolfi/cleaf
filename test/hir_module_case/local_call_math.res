Function math__helper
0: t1 = INT_CONST 1
1: MOV t-1 t1
2: RETURN
Function math__add
0: CALL math__helper
1: MOV t1 t-1
2: MOV t-1 t1
3: RETURN
