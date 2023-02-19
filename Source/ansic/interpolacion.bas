1 rem ***********************
2 rem **   INTERPOLACION   **
3 rem **  Autores.-        **
4 rem *  F. Botana Ferreiro *
5 rem *    y Cecilia Peña   *
6 rem *  - Julian Albo -    * Modificaciones y adaptacion a Blassic.
7 rem * (C) Los autores y   *
8 rem *El Ordenador Personal* Num. 34, Marzo 1985
9 rem ***********************
10 poke sysvarptr + 25, 1 ' VAR evaluate expressions.
15 goto 100
20 let j= (x - x(1)) * 245 / (x(n)-x(1)) + 6
30 let k= (y - my) * 148 / (ym-my) + 16
40 for m= j - 1 to j + 1: plot m, k: next m
50 for m= k - 1 to k + 1: plot j, m: next m
60 return
80 for j= 1 to i - 1: let b$= b$ + "*(x-x("+str$(j)+"))": next j
90 return
99 rem
100 cls: input "Cuantos puntos?"; n: if n < 2 then goto 100
110 dim f(n)
120 dim x(n): dim y(n)
130 input "Abscisas equidistantes (s/n)? "; r$
140 if r$ = "s" then input "Equidistancia? "; h: if h = 0 then goto 140
150 input "x(1), y(1): "; x(1), y(1): print "X", "Y",,,x(1), y(1)
160 for i= 2 to n
170 if r$ <> "s" then print "x"; i; ", y"; i; ": ";: input x(i), y(i): goto 190
180 let x(i)= x(1) + h * (i-1): print "y"; i; ": ": input y(i)
190 print x(i), y(i)
200 next i
210 cls: print at 11, 10; "Calculando"
220 for i= 1 to n - 1
230 for j= i + 1 to n
240 if x(i) > x(j) then let l= x(i): let m= y(i): let x(i)= x(j): let y(i)= y(j): let x(j)= l: let y(j)= m
250 next j
260 next i
270 let f(1)= y(1)
280 for r= 2 to n
290 let c= 0
300 for i= 1 to r
310 let d= 1
320 for j= 1 to r
330 if j = i then goto 350
340 let d= d * (x(i) - x(j) )
350 next j
360 let c= c + y(i) / d
370 next i
380 let f (i-1)= c
390 next r
400 let a$="0"
410 for i= 1 to n
420 let b$= "": if i > 1 then gosub 80
430 let a$= a$+"+f("+str$ (i)+")"+b$
440 next i
450 def fn b(x)= val (a$)
460 let ym= y (1): let my= y(1)
470 for i= 2 to n
480 if ym < y(i) then let ym= y(i)
490 if my > y(i) then let my= y(i)
500 next i
510 for i= x(1) to x(n) step (x(n)-x(1)) / 60
520 let y= fn b(i)
530 if ym < y then let ym= y
540 if my > y then let my= y
550 next i
555 rem
560 mode "spectrum": if x(1)*x(n) <= 0 then plot -x(1) * 245 / (x(n) - x(1) ) + 6, 16: drawr 0, 148
570 if ym * my <= 0 then plot 6, -my * 148 / (ym-my) + 16: drawr 245, 0
580 if ym - my = 0 then mode 0: print at 10, 8; "Poca informacion!": stop
590 for i= 1 to n
600 let x= x(i): let y= y(i): gosub 20: circle j, k, 2
610 next i
700 for i= x(1) to x(n) step (x(n)-x(1)) / 60
710 let x= i: let y= fn b(i)
720 gosub 20
730 next i
1000 get key$
