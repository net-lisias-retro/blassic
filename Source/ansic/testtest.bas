      5 MODE 10
     10 INK 0, 32, 235, 192
     20 INK 15, 221, 37, 63
     30 LOCATE 1, 1
     40 PRINT "Hola"
     50 ink 0, 0
     60 ink 15, 26
     70 ink 1, 32, 235, 192
     80 ink 2, 221, 37, 63
    100 LOCATE 4, 1
    110 FOR i= 0 TO 7
    120         FOR j= 0 TO 7
    130                 PRINT TEST (j, i),
    140         NEXT
    150         PRINT
    160 NEXT
    200 GET a$
