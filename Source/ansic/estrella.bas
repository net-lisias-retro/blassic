      1 REM Estrella
      2 REM --------
      3 REM (C) R. Beczkowski
      4 REM Original para ZX-81, El Ordenador Personal N. 29, 1984
     10 PRINT "Cuantos brazos para su estrella ?"
     20 INPUT a
     25 MODE 1
     30 FOR u= 0 TO 12 * a
     40 PLOT 32 + (15+6*COS (PI*u/6))* COS (PI*u/(a*6)), 22 + (15 + 6 * COS (PI*u/6)) * SIN (PI*u/(a*6))
     50 NEXT u
