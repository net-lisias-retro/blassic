     10 REM
     20 REM
     30 REM Integrales - Aceleracion por el metodo de Euler.
     40 REM
     50 REM (C) Andre Warusfel y El Ordenador Personal
     60 REM
     70 REM
     80 rem def int g, i, j, k, n
     90 n= 500: a= 0: h= PI / n
    100 DIM e (n): s= 0: e= 0
    110 FOR i= 0 TO n
    120 	GOSUB 210: e (i)= u
    130 	PRINT i + 1;: s= s + u: PRINT "", s;
    140 	IF i = 0 THEN GOTO 180
    150 	FOR j= i - 1 TO 0 STEP -1
    160 		e (j)= (e (j) + e (j+1) ) / 2
    170 	NEXT j
    180 	e= e + e (0) / 2: PRINT "", e
    190 NEXT i
    200 PRINT: END
    210 b= a + PI
    220 x= a: GOSUB 320
    230 u= y: g= 2
    240 FOR k= 1 TO n
    250 	g= 6 - g: x= x + h
    260 	GOSUB 320
    270 	u= u + g * y
    280 NEXT k
    290 u= h * (u - y) / 3
    300 a= b
    310 RETURN
    320 IF x < 1E-2 THEN y= 1 - x * x / 6 ELSE y= SIN (x) / x
    330 y= y + y
    340 RETURN
