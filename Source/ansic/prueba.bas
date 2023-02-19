      1 GOTO 10
      2 GOTO 2010
     10 REM
     50 ON ERROR GOTO 160
    100 posgosub= PROGRAMPTR + 28
    110 INPUT "What line"; l
    120 IF l = 0 THEN END
    130 POKE32 posgosub, l
    140 GOSUB 2
    150 GOTO 110
    160 IF ERR <> 45 OR ERL <> 2 THEN PRINT STRERR$ (ERR); " on line "; ERL: END
    170 PRINT "That line not exist, try other"
    180 RESUME 190
    190 RETURN
   1000 PRINT 1000
   1010 RETURN
   2000 PRINT 2000
   2010 RETURN
