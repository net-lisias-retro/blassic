      5 MODE 1
     10 REPEAT
     20 	a$= INKEY$
     30 	LOCATE 1, 1: PRINT using "###", XMOUSE
     40 	LOCATE 2, 1: PRINT using "###", YMOUSE
     50 UNTIL a$= "CLICK"
