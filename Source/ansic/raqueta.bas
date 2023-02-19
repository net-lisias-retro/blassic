     10 CLS
    100 ypos= 25
    110 xpos= 4
    120 xposmin= 2
    130 xposmax= 77
    135 esc$= chr$ (27)
    137 raq$= " -- "
    140 GOSUB 20000
    150 END
  10000 REM Pon la raqueta
  10010 LOCATE ypos, xpos - 1
  10020 PRINT raq$;
  10030 RETURN
  20000 GOSUB 10000
  20010 a$= upper$ (INKEY$)
  20020 IF a$ = "M" OR a$ = "RIGHT" THEN xpos= xpos + 1
  20030 IF a$ = "N" OR a$ = "LEFT" THEN xpos= xpos - 1
  20040 rem IF xpos < xposmin THEN xpos= xposmin
  20050 rem IF xpos > xposmax THEN xpos= xposmax
  20055 xpos= max (min (xpos, xposmax), xposmin)
  20060 IF a$ = "Q" OR a$ = esc$ THEN RETURN
  20070 GOTO 20000
