1 rem *******************************
2 rem ***        FRACTALE         ***
3 rem ***    la funcion recursiva ***
4 rem ***                         ***
5 rem ***    El Ordenador Personal*** Num 46 Marzo 1986
6 rem ***             y           ***
7 rem ***      Max HAGENBURGER    ***
8 rem *******************************
9 rem Original para Oric-1 adaptado por Julian Albo
10 gosub 100
20 gosub 50 : if dess1n then 20
30 gosub 900
40 rem -------
50 gosub 200: rem dibujo
60 gosub 300: rem fractal
70 gosub 800: return
90 rem ======
100 rem inicializacion
110 gosub 700
120 dim alea (19)
130 print "Funcion FRACTAL, recursividad del"
140 print "fraccionamiento aleatorio (ejemplo:0)"
150 prof$="0"
160 randomize
190 return: rem -------
200 rem comienzo del dibujo
210 prof= val (prof$)
220 x1= 10:     y1= 130
230 x2= 220:    y2= 145
240 x3= 120:    y3= 25
250 for i= 0 to 19
260 alea (i)= rnd (1)
270 next
290 return: rem -----
300 rem FRACTAL (recursiva)
310 if prof = 0 then gosub 400: return
320 pil= pil + 1
330 gosub 500: rem apilar y segmento
340 x1= x1 (pil): y1= y1 (pil): x2= x4 (pil): y2= y4 (pil): x3= x6 (pil): y3= y6 (pil)
345 prof= prof (pil) - 1: gosub 300
350 x1= x2 (pil): y1= y2 (pil): x2= x5 (pil): y2= y5 (pil): x3= x4 (pil): y3= y4 (pil)
355 prof= prof (pil) - 1: gosub 300
360 x1= x3 (pil): y1= y3 (pil): x2= x6 (pil): y2= y6 (pil): x3= x5 (pil): y3= y5 (pil)
365 prof= prof (pil) - 1: gosub 300
370 x1= x4 (pil): y1= y4 (pil): x2= x5 (pil): y2= y5 (pil): x3= x6 (pil): y3= y6 (pil)
375 prof= prof (pil) - 1: gosub 300
380 pil= pil - 1
390 return: rem ------
400 rem trazado de un triangulo (/ordinario.)
410 plot x1, y1 to x2, y2 to x3, y3 to x1, y1
490 return: rem ------
500 rem apilar y segmento
510 prof (pil)= prof
520 x1 (pil)= x1: y1 (pil)= y1
530 x2 (pil)= x2: y2 (pil)= y2
540 x3 (pil)= x3: y3 (pil)= y3
550 rem mitad de los tres lados
560 xa= x1: ya= y1: xb= x2: yb= y2: gosub 600
565 x4 (pil)= xm: y4 (pil)= ym
570 xa= x2: ya= y2: xb= x3: yb= y3: gosub 600
575 x5 (pil)= xm: y5 (pil)= ym
580 xa= x3: ya= y3: xb= x1: yb= y1: gosub 600
585 x6 (pil)= xm: y6 (pil)= ym
590 return: rem -------
600 rem segmentacion
610 l= abs (xa - xb) + abs (ya - yb)
620 k= (xa + xb) * 2 + ya + yb
630 dist= (alea (k - int (k/20) * 20) - .5) / 5
640 xm= int (dist * l + (xa + xb) / 2)
650 k= (ya + yb) * 2 + xa + xb
660 dist= (alea (k - int (k/20) * 20) - .5) / 5
670 ym= int (dist * l + (ya + yb) / 2)
690 return: rem -------
700 rem borrar pantalla (/ordenador)
710 mode 1
790 return: rem ------
800 rem fin de una orden
810 print chr$ (7);
820 print "Profundidad de 0 a 6 o 'F' ?";
830 get prof$: if (prof$ < "0" or prof$ > "6") and prof$ <> "F" then 830
840 dess1n= (prof$ <> "F")
850 if dess1n then gosub 700: print prof$
890 return: rem --------
900 print: print "fin de programa";
990 end: rem ============
