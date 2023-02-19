     10 OPEN "muchavar.bas" FOR OUTPUT AS #1
     15 n= 10000
     20 nline= 10
     30 FOR i= 1 TO n
     40 	PRINT #1, STR$(nline) + " a" + STR$ (i) + "= " + STR$ (i)
     45		print chr$(13); i;
     50 	nline= nline + 10
     60 NEXT
     70 PRINT
     80 CLOSE 1
