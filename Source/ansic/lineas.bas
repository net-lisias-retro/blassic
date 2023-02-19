      5 d= sysvarptr
      6 print peek (d + 4) + 256 * peek (d + 5)
      7 m= val (programarg$ (1) ) : if m = 0 then m= 1
      8 h= val (programarg$ (2) )
     10 if h = 0 then mode m else mode m, h
     15 w= peek (d) + 256 * peek (d + 1)
     16 h= peek (d + 2) + 256 * peek (d + 3)
     20 FOR i= 0 TO h STEP 10
     30 MOVE 0, i
     40 draw w - 1, i
     50 NEXT
     52 MOVE 0, h -1 : draw w - 1, h - 1
     54 MOVE 0, 0 : draw 0, h - 1
     56 MOVE w - 1, 0 : draw w - 1, h -1
     57 PLOT 0, 0 TO w - 1, h - 1
     58 PLOT w - 1, 0 TO 0, h - 1
     60 get a$
     70 MODE 0
