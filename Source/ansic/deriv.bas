1 rem deriv.bas
2 rem Autor: Diego Soriano, El Ordenador Personal 42, Noviembre 1985
3 rem Original escrito para MSX
4 rem Adaptacion: Julian Albo
5 rem **** NO FUNCIONA. Si alguien encuentra el error por favor, comuníquemelo****
10 clear
20 dim fp$ (100), mu$ (15)
30 cls
40 print "Introduzca la función y pulse RETURN": print: input a$
50 a= len (a$): c= 1
60 dim m (a + 5)
70 dp$= a$:  goto 110
80 rem ******Bucle principal*******
90 if fi = 0 then print "-- Función derivada en proceso --": print: for q= 0 to g: print fp$ (q);: next q: print: print
100 c= 1: gosub 1070
110 if fi = 1 then print "Final del proceso": print: print "Función inicial y="; a$: print string$ (40, "-"): print: print "Función derivada ": print "y'=";: for q= 0 to g: print fp$ (q);: next q: print: print string$ (40, "-")
120 if fi = 1 then end
130 dp= len (dp$)
140 d= dp: d$= dp$: gosub 290
150 d= dp: d$= dp$: gosub 450
160 if fd = 1 then fd= 0: gosub 1670: fu$= t$: gosub 630:pa$="": goto 230
170 if fm = 1 then fm= 0: fu$= t$: gosub 630: pa$= "": gosub 1390: goto 230
180 d= len (t$): d$= t$: gosub 560
185 if cr = 1 then cr= 0: goto 230
190 if t$ = "" then sd= 0: gosub 1200: goto 90
200 if ok >= 1 and pa$ = "" then p1$= p1$ + "(": kl= 1: goto 110 else goto 110
220 gosub 680
221 print"de$="; de$
222 if de$ = "1" and sp = 1 and sg$ <> "-" then p1$= left$ (p1$, len (p1$) - 1): de$= "": goto 150
225 if sp = 1 and ok >= 1 then sp= 0: p1$= p1$ + "(": kl= 1: ok= 0
230 if de$ <> "" then if sg$ = "-" or sd = 1 then  p1$= p1$ + sg$ + de$ else p1$= p1$ + de$
240 if pa$ <> "" then p1$= p1$ + "*D" + pa$ + "D"
250 de$= "": goto 150
260 end
270 rem ***********Rutinas************
280 rem ***********Rutinas de codificación************
290 p= 0: i= 0: ok= 0: for t= 1 to a: m (t)= 0: next t: ok= 0
300 for t= 1 to d
310 if t > 1 then lm= 1 else lm= 0
320 p$= mid$ (d$, t, 1): mi$= mid$ (d$, t - lm, 1)
330 if p$ = "(" then p= p + 1: i= 0: goto 410
340 if p$ = ")" then m (t)= p + 0.5: p= p - 1: goto 420
350 if p$ = "X" then m (t)= 0.9: goto 420
360 if p$ = "D" then m (t)= 0.7: goto 420
370 if p$ = "+" or p$ = "-" and mi$ <> "*" and mi$ <> "^" and t > 1 and mi$ <> "(" then i= 0.25: ok= ok + 1: goto 410
380 if p$ = "*" then i= 0.35: goto 410
390 if p$ = "/" then i= 0.5: goto 410
400 m (t)= 0: goto 420
410 m (t)= p + i
420 next t
430 return
440 rem **********Rutina separación de términos*********
450 for t= 1 to d
460 if m (t) = 0.25 and t > c then goto 500
470 if m (t) = 0.35 and m (t + 1) <> 0.7 and m (t - 1) <> 0.7 then fm= 1
480 if m (t) = 0.45 then if m (t - 2) <> 0.9 then fd= 1
490 next t
500 t$= mid$ (d$, c, t -c)
510 i1= c - 1: i2= t
520 if i2 > len (t$) then i2= len (t$)
530 c= t
540 return
550 rem ********Rutina separción de paréntesis y función*********
560 for t= 1 to d
570 p$= mid$ (d$, t, 1)
580 if p$ = "(" then goto 610
590 next t
600 fu$= d$: pa$= "": goto 630
610 fu$= left$ (d$, t - 1)
620 pa$= mid$ (d$, t + 1, d - t -1)
630 sg$= left$ (fu$, 1)
631 if sg$ = "+" or sg$ = "-" then pk= 2 else pk= 1
635 if fu$ = sg$ and val (right$ (d$, 1)) <> 0 then gosub 661: goto 650
640 if sg$ = "+" or sg$ = "-" then fu$= right$ (fu$, len (fu$) - 1)
650 if (sg$ <> "+" and sg$ <> "-") then sg$= ""
660 return
661 for t= len (d$) to 1 step -1: p$= mid$ (d$, t, 1)
662 if p$= "^" then ex$= right$ (d$, d - t): pa$= left$ (d$, t - 2): pa$= right$ (pa$, len (pa$) - pk): goto 664
663 next t
664 ex= val (ex$): ep= ex - 1: ep$= str$ (ep): if ep > 0 then ep$= right$ (ep$, len (ep$) - 1)
666 if ep$ <> "1" then de$= ex$ + "*(" + pa$ + ")^" + ep$ else de$= ex$ + "*(" + pa$ + ")"
668 cr= 1
669 return
670 rem ***********Rutina de derivación***************
680 de$= "": if fu$= "LN" then de$= "1/(" + pa$ + ")": return
690 if fu$ = "COS" then de$= "SEN(" + pa$ + ")": if sg$ = "+" or sg$ = "" then sg$= "-": return else sg$= "+": return
700 if fu$ = "SEN" then de$= "COS(" + pa$ + ")": return
710 if fu$ = "TG" then de$= "(1+(TG(" + pa$ + ")^2)": return
720 if fu$ = "ARCSEN" then de$= "1/1-(" + pa$ + ")^2)^.5": return
730 if fu$ = "CTG" then de$= "(1+CTG(" + pa$ + ")^2)": return
740 if fu$ = "ARCOS" then de$= "-1/(1-(" + pa$ + ")^2)^.5": return
750 if fu$ = "ARCTG" then de$= "1/(1+(" + pa$ + ")^2)": return
760 if right$ (fu$, 1) = "^" then de$= t$ + "*" + "LN" + "(" + left$ (fu$, len (fu$) - 1) + ")": return
770 if val (left$ (fu$, 1) ) <> 0 or left$ (fu$, 1) = "X" then fu$= sg$ + fu$: gosub 810: return
780 print fu$; " no es variable."
785 print "Algo es incorrecto, adios": end
790 return
800 rem *********Rutina derivacion polinomica**********
810 fu= len (fu$): gr$= "": co$= ""
820 for p= 1 to fu
830 p$= mid$ (fu$, p, 1)
840 if p$ = "X" then co$= left$ (fu$, p - 1): al= p: goto 870
850 next p
860 return
870 om= 0: for p= fu to 1 step -1
880 p$= mid$ (fu$, p, 1)
890 if p$ = "^" then gr$= right$ (fu$, fu - p): om= p - 1: goto 910
900 next p
910 h$= mid$ (fu$, al, al - om + 1)
920 gr= val (gr$): if gr = 0 then gr= 1
930 co= val (co$)
940 if co = 0 then if sg$ = "+" or sg$ = "" then co= 1 else co= -1
950 cp= (co * gr)
960 sg= sgn (cp): if sg < 0 then sg$= "-" else sg$= "+"
970 cp= abs (cp)
980 gp= gr - 1
990 cp$= str$ (cp): if cp >= 0 then cp$= right$ (cp$, len (cp$) - 1)
1000 gp$= str$ (gp): if gp >= 0 then gp$= right$ (gp$, len (gp$) - 1)
1010 de$= cp$
1020 if gp$ <> "0" then de$= de$ + h$
1030 if gp$ <> "1" and gp$ <> "0" then de$= de$ + "^" + gp$
1040 if h$ <> "X" then de$= de$ + "*" + "D" + h$ + "D"
1050 return
1060 rem ********Rutina de separación derivadas parciales******
1070 for v= 0 to g: fp= len (fp$ (v)): fg= 0
1080 for t= 1 to fp
1090 p$= mid$ (fp$ (v), t, 1)
1100 if p$ = "D" and fg = 0 then fg= 1: s= t: goto 1120
1110 if p$ = "D" and fg = 1 then r= t: fg= 0: goto 1150
1120 next t
1130 next v
1140 fi= 1: return
1150 p1$= left$ (fp$ (v), s - 1)
1160 dp$= mid$ (fp$ (v), s + 1, r - s - 1)
1170 p2$= right$ (fp$ (v), fp - r): if left$ (p2$, 1) = "*" then fw= 1
1175 if right$ (p1$, 1)  = "*" or right$ (p1$, 1) = "/" then sp= 1 else sp= 0
1180 return
1190 rem **********Rutina archivo y unión*********
1200 fp= len (fp$ (v) )
1210 p1= len (p1$)
1220 p2= len (p2$)
1230 if kl = 1 then kl= 0: p1$= p1$ + ")"
1240 if p1 + p2 > 200 and p1 + p2 > p1 and p2$ <> fp$ (v + 1) then goto 1280
1250 fp$ (v)= p1$
1260 if p2$ <> fp$ (v + 1) then fp$ (v)= fp$ (v) + p2$
1270 return
1280 gosub 1330
1290 fp$ (v)= p1$
1300 fp$ (v + 1)= p2$
1310 return
1320 rem ********Rutina movimiento de fp$ **************
1330 for ex= g + 1 to v + 2 step -1
1340 fp$ (ex)= fp$ (ex - 1)
1350 next ex
1360 fp$ (v + 1)= "": g= g + 1
1370 return
1380 rem *********Rutina multiplicación de funciones********
1390 p= i1 + 1: j= 0: k2= 0
1400 for t= 1 to len (dp$)
1410 if m (t) = 0.35 then m (t)= - m (t): goto 1440
1420 next t
1430 t= len (t$) + i1 + 1
1440 r$= mid$ (dp$, p, t - p): p= t + 1
1450 mu$ (j)= r$
1460 j= j + 1
1470 if p > len (t$) then j= j - 1: goto 1490
1480 goto 1400
1490 rp= val (mu$ (k2) ): rp$= str$ (rp): if rp > 0 then rp$= right$ (rp$, len (rp$) - 1)
1500 if rp$ = mu$ (k2) then k2= k2 + 1: goto 1490
1510 h$= "D" + mu$ (k2) + "D"
1520 for t= 0 to j
1530 if t = k2 then goto 1550
1540 i$= i$ + "*" + mu$ (t)
1550 next t
1560 if k2 = 0 then de$= h$ + i$ else de$= de$ + "+" + h$ + i$: i$= "": h$= ""
1565 k2= k2 + 1: h$= "": i$= ""
1570 if k2 > j then 1590
1580 goto 1490
1590 if left$ (de$, 1) = "+" then de$= right$ (de$, len (de$) - 1)
1595 if sp = 1 then sp= 0: de$= "(" + de$ + ")"
1600 sg$= ""
1610 return
1660 rem *********Rutina derivación división de funciones******
1670 for t= 1 to len (dp$)
1680 if m (t) = 0.45 then m (t)= - m (t): goto 1700
1690 next t
1700 uf= m (t - 1): di= m (t + 1)
1710 for p= t - 1 to 1 step -1
1720 if m (p) = int (uf) then goto 1740
1730 next p
1740 dn$= mid$ (dp$, p + 1, t - p - 2)
1750 for p= t + 2 to len (dp$)
1760 if m (p) - di = 0.5 then goto 1780
1770 next p
1780 dv$= mid$ (dp$, t + 2, p - t - 2)
1790 de$= "(" + "D" + dn$ + "D*" + dv$ + "-" + dn$ + "*D" + dv$ + "D" + ")/(" + dv$ + ")^2"
1810 return
