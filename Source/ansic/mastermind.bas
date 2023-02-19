1 rem MasterMind
2 rem Autor: Jesus Glez Nores
3 rem Copyright: El autor y El Ordenador Personal (Num 12, enero-febrero 1983)
4 rem Original para ZX-81 adaptado por Julian Albo
5 rem **********************
6 rem Modalidad de juego
7 rem **********************
8 rem Activa el modo ZX de comprobar los NEXT, pemite DIM repetidos.
9 poke sysvarptr + 26, 1: poke sysvarptr + 27, 1 ' NEXT relaxed, can reDIM.
10 print "Numero de jugadores (hasta 12)?"
20 input x
30 cls
35 if x > 12 then goto 10
40 print "Numero de cifras (hasta 10)?"
50 input y
60 cls
65 if y > 10 then goto 40
66 if y > 6 then goto 68
67 goto 70
68 let z$= "SIN"
69 goto 120
70 print "Con repeticion pulsar ""CON"""
80 print
90 print "Sin repeticion pulsar ""SIN"""
100 input z$
105 z$= upper$ (z$)
110 cls
113 if z$ <> "CON" and z$ <> "SIN" then goto 70
114 rem **********************
115 rem Puesta a cero y salida
116 rem **********************
120 dim p(x)
130 for n= 1 to x
140 let p (n)= 0
150 next n
155 randomize time
160 for t=1 to x
170 cls
180 for n= 1 to 40
190 print at 11, 11; "Turno de """; chr$ (asc("0") + t);""""
195 pause 50
200 print at 11, 11; "            "
205 pause 50
210 next n
220 print at 13, 2; """"; Y; """ cifras """; z$; """ repeticion"
230 gosub 3000
240 dim x (y)
250 dim a (y)
260 if z$ = "SIN" then goto 310
264 rem ************************
265 rem Clave con repeticion
266 rem ************************
270 for n= 1 to y
280 let x (n)= int (rnd * 10)
290 next n
300 let r= u - 4 * int (u/4)
304 rem **************************
305 rem Clave sin repeticion
306 rem **************************
310 for n= 1 to y
320 let x (n)= int (rnd * 10)
325 repite= 0
330 for m= 1 to n
340 if m = n then goto 360
350 if x (m) = x (n) then goto 320
360 next m
370 next n
380 let p= 0
384 rem *******************
385 rem Jugadas
386 rem *******************
390 print at 14, 0; : input x$: print at 14, 0; "                   "
400 if len (x$) <> y then goto 420
410 goto 455
420 for z= q to 15
430 print at p, 0; x$
435 pause 50
440 print at p, 0; "                "
442 pause 50
444 print at 13, 3; " "
445 pause 50
446 print at 13, 3; y
447 pause 50
450 next z
452 goto 390
455 if z$ = "SIN" then goto 461
456 goto 479
461 for m= 1 to y
462 let a (m)= val (mid$ (x$, m, 1) )
463 next m
464 for m= 1 to y
465 for n= 1 to y
466 if m = n then goto 476
467 if a (m) = a (n) then goto 469
468 goto 476
469 for z= 1 to 30
470 print at p, 0; x$: pause 50
471 print at p, 0; "           ": pause 50
472 print at 13, 12; "       ": pause 50
473 print at 13, 12; " ""SIN"" ": pause 50
474 next z
475 goto 390
476 next n
477 next m 
479 let mu= 0
480 print at p, 0; x$;
490 let p= p + 1
495 rem **************************
496 rem Muertos y heridos
497 rem **************************
500 for m= 1 to y
510 let a (m)= val (mid$ (x$, m, 1) )
520 for n= 1 to y
530 if a (m) = x (n) and m = n then goto 550
540 goto 580
550 let mu= mu + 1
560 print " M";
570 rem if mu = y then goto 630
580 if a (m) = x (n) and m <> n then print " h";
590 next n
600 next m
602 if mu >= y then goto 630
604 rem *************************
605 rem Final de la partida
606 rem *************************
610 if p = 10 then goto 625
620 goto 390
625 let p= 12
630 print
640 print "CLAVE"
650 for n= 1 to y
660 print x (n);
670 next n
674 rem *******************
675 rem Puntuaciones
676 rem *******************
680 let p (t)= p (t) + p
690 gosub 3000
700 print at 21, 0; "Para seguir jugando, pulsar ""1"""
710 input s
715 if s <> 1 then goto 700
720 next t
730 goto 160
3000 for u= 0 to x - 1
3010 let r= u - 4 * int (u/4)
3020 let l= 16
3030 if u > 3 then let l= 18
3040 if u > 7 then let l= 20
3050 print at l, 8 * r; chr$ (asc ("1") + u); "= "; p (u + 1);
3060 next u
3070 return
