     10 REM
     20 LABEL aqui : FOR i= 1 TO 10
     30 PRINT i
     40 NEXT i
     50 GOTO alli
     60 PRINT "Otra vuelta"
     70 LABEL alli
     80 j= j + 1
     90 IF j < 2 THEN GOTO aqui
    100 END
