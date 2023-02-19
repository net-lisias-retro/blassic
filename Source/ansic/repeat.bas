     10 REM repeat test
     20 REPEAT
     30         PRINT "Otra"
     40         REPEAT
     50 	        a$= INKEY$
     60         UNTIL a$ <> ""
     70 UNTIL a$ = "f" OR a$ = "F"
