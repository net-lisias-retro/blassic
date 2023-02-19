     10 REM La jaca Paca va para Alava
     20 GOTO 9000
    980 REM
   1000 r= PEEK (a) + 256 * (PEEK (a+1) + 256 * (PEEK (a+2) + 256 * PEEK (a+3) ) )
   1010 RETURN
   9000 REM Inicio
   9010 p= PROGRAMPTR
   9020 a= p : GOSUB 1000 : n= r
   9030 IF n <> 10 THEN END
   9040 a= p + 4 : GOSUB 1000 : l= r
   9050 IF l <= 0 THEN END
   9060 FOR i= p + 8 TO p + 7 + l
   9070 	c= PEEK (i)
   9080 	IF c = ASC ("a") THEN POKE i, ASC ("i")
   9085 	IF c = ASC (" ") THEN POKE i, ASC ("-")
   9090 NEXT
   9100 LIST
   9110 END
