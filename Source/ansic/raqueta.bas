     10 CLS
    100 posy= 25
    110 posx= 4
    120 posxmin= 2
    130 posxmax= 77
    135 esc$= chr$ (27)
    137 raq$= " -- "
    140 GOSUB 20000
    150 END
  10000 REM Pon la raqueta
  10010 LOCATE posy, posx - 1
  10020 PRINT raq$;
  10030 RETURN
  20000 GOSUB 10000
  20010 a$= upper$ (INKEY$)
  20020 IF a$ = "M" OR a$ = "RIGHT" THEN posx= posx + 1
  20030 IF a$ = "N" OR a$ = "LEFT" THEN posx= posx - 1
  20055 posx= max (min (posx, posxmax), posxmin)
  20060 IF a$ = "Q" OR a$ = esc$ THEN RETURN
  20070 GOTO 20000
