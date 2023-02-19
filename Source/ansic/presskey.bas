     10 REPEAT
     30 	a$= INKEY$
     40 UNTIL a$ <> ""
     50 a= ASC (a$)
     60 IF LEN (a$) > 1 OR (a > 31 AND a < 127) THEN PRINT a$ ELSE PRINT a
     70 GOTO 10
