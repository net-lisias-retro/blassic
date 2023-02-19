     10 MODE 1
     20 v= 199: h= 319
     75 color , 0 : cls
     80 rem for i= 1 to 50000: next
    120 FOR c= 0 TO 15
    130 y= c * 10 + 1
    140 COLOR c
    150 MOVE 0, y: draw h, y
    160 move 0, y + 1: draw h, y + 1
    170 NEXT
    200 get a$
    300 MODE 0
    310 rem exit
