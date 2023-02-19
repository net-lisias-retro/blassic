10 rem ************************************
20 rem *                                  *
30 rem *        Graficos 3-D Basic        *
50 rem *     Version 1.1 / Septiembre     *
60 rem *      El Ordenador Personal       *
70 rem *         y  Jean Safar            *
90 rem ************************************
94 rem Original para Amstrad CPC publicado en
96 rem El Ordenador Personal num. 56, febrero 1987
98 rem Adaptado a Blassic y animado por Julian Albo.
100 rem
110 rem on error goto 1080
170 dim x (60), y (60), z (60), n (100)
180 dim u (60), v (60), w (60), mm (30)
190 rem
200 rem ------- Lectura de data
210 rem
220 read ee, js
230 mm= 0
240 for i= 1 to ee
250 read mm (i)
260 mm= mm + mm (i)
270 next i
280 for i= 1 to mm
290 read n (i)
300 next i
310 for i= 1 to js
320 read x (i), y (i), z (i)
330 next i
335 read final$: if final$ <> "final" then error 60000
340 orgx=320: orgy= 210
360 rem
370 rem -------- Intro. angulos y distancias
380 rem
390 input "Distancia observador: "; r
400 if r = 0 then end
410 input "Theta: "; t
420 if t >= 361 then goto 390
425 t= t * 2 * pi / 360
430 input "Phi: "; f
440 if f >= 361 then goto 390
445 f= f * 2 * pi / 360
450 input "Distancia pantalla: "; a
460 if a = 0 then goto 390
470 rem
480 rem -------- Algoritmo 3-D
490 rem
495 inc= pi / 180
497 mode "cpc2"
498 synchronize 1
500 c1= cos (f): c2= cos (t): s1= sin (f): s2= sin (t)
510 for i= 1 to js
520 u (i)= -x (i) * s2 + y (i) * c2
530 v (i)= -x (i) * c2 * s1 - y (i) * s2 * s1 + z (i) * c1
540 w (i)= -x (i) * c2 * c1 - y (i) * s2 * c1 - z (i) * s1 + r
550 next i
560 rem
570 rem -------- Trazado del objeto
580 rem
585 cls
590 l= 1: for i= 1 to ee
600 plot orgx + a * u (n (l) ) / w (n (l) ), orgy + a * v (n (l) ) / w (n (l) )
610 for j= l + 1 to l + mm (i) - 1
620 draw orgx + a *  u (n (j) ) / w (n (j) ), orgy + a * v (n (j) ) / w (n (j) )
630 next j
640 draw orgx + a * u (n (l) ) / w (n (l) ), orgy + a * v (n (l) ) / w (n (l) )
650 l= l + mm (i)
660 next i
665 synchronize
670 get tecla$: if tecla$ = " " then mode 0: goto 340
671 if tecla$ = "LEFT" then t= t + inc: goto 500
672 if tecla$ = "RIGHT" then t= t - inc: goto 500
673 if tecla$ = "UP" then f= f - inc: goto 500
674 if tecla$ = "DOWN" then f= f + inc: goto 500
675 if upper$ (tecla$) = "A" then a= a + 5: goto 500
676 if upper$ (tecla$) = "Z" then a= a - 5: a= max (a, 10): goto 500
677 if upper$ (tecla$) = "S" then r= r + 5: goto 500
678 if upper$ (tecla$) = "X" then r= r - 5: r= max (r, 10): goto 500
679 goto 670
680 rem
690 rem -------- Datos
700 rem
710 data 24,49,4,4,4,4,4,4,4,4,4,4,4,3,3,3,4,4,4,4,4,4,4,4,4,4
720 data 2,6,5,1,3,7,6,2,4,8,7,3,1,5,8,4,6,10,9,5,8,9,10,7
730 data 12,6,5,11,13,7,6,12,14,8,7,13,11,5,8,14
740 data 16,17,18,15,16,21,17,21,20,17,18,20,19,22,23,27,26,25,24,27,26,30,29,28,31
750 data 39,33,32,36,38,34,33,39,37,35,34,38,36,32,35,37
760 data 41,42,43,40,44,45,46,47,45,48,49,46
770 rem
780 rem ------- Datos de la casa
790 rem
800 data 20,-27,-20,20,27,-20,-20,27,-20,-20,-27,-20
810 data 20,-27,0,20,27,0,-20,27,0,-20,-27,0
820 data 0,-17,20,0,17,20
830 data 23,-30,-2,23,30,-2,-23,30,-2,-23,-30,-2
840 data 20,-5,0,20,5,0,20,5,7,20,-5,7,13,-5,7
850 data 10,0,10,13,5,7,20,4,0,20,4,7,20,0,7
860 data 20,0,0,20,-4,0,20,-4,7,20,-4,5,20,4,5,20,4,2,20,-4,2
870 data -2,-15,22,-2,-12,22,-5,-12,22,-5,-15,22
880 data -2,-15,18,-5,-15,15,-5,-12,15,-2,-12,18
890 data 40,-47,-20,40,47,-20,-40,47,-20,-40,-47,-20
900 data 40,6,-20,20,3,-20,20,-3,-20,40,-6,-20
910 data 20,3,-5,20,-3,-5
915 data final
