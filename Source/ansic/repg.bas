5 rem Programa repg.bas
10 rem Resolucion de ecuaciones de primer grado.
20 rem Miguel A. Lerma
30 rem Publicado en El Ordenador Personal num. 24, 1984
40 rem Original para Spectrum adaptado a Blassic por J. Albo
70 print
80 print "Este programa sirve para resolver ecuaciones de primer grado.": print "Siga las instrucciones."
90 print
100 rem
110 poke sysvarptr + 25, 1 ' VAL evaluate expressions
120 print
200 rem Introduccion de la ecuacion
230 print "Intruduzca la ecuacion usando x como incognita, ""s"" si desea detener el programa": line input a$
240 if a$ = "s" then stop
300 rem Analisis y puesta a punto de la ecuacion
310 let l= len (a$): if l = 0 then goto 100
320 let b$= a$
340 for n= 1 to l
350 if mid$ (b$, n, 1) = "=" then b$= left$ (b$, n - 1) + "-(" + mid$ (b$, n + 1) + ")": l= l + 2
360 next n
380 def fn f (x)= val (b$)
400 rem Control de linealidad
420 for n= 10 to 1 step -1
430 let a= fn f (n - 1): let b= fn f (n): let c= fn f (n + 1)
440 if abs (a + c - 2 * b) > .0001 then print "La expresion", , a$: print "no es una ecuacion de primer grado.": goto 100
450 next n
500 rem Resolucion de la ecuacion
510 if a = b then print "La ecuacion", , a$: print "no tiene solucion.": goto 100
520 let x0= a / (a - b)
530 print "La solucion de la ecuacion", a$: print "es x= "; x0
600 rem Calculo de la solucion en forma fraccionaria
620 let x1= abs (x0)
630 let e= .000001 * (x1 + 1)
640 let p= 0: let q= 1
650 let r= x1 - p / q
660 if r < -e then let q= q + 1: goto 650
670 if r > e then let p= p + 1: goto 650
700 rem
720 print "y en forma fraccionaria es", "x= ";
730 if x0 < 0 then print "-";
740 print p; "/"; q
750 goto 100
