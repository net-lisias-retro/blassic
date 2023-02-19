1 rem 4raya.bas
2 rem Autor: Thierry Lévy-Abegnoli
3 rem Revista El Ordenador Personal Nº 34, marzo 1985.
5 rem Terminado de copiar pero no juega bien, seguramente hay errores.
7 poke sysvarptr + 26, 1 : rem Comprobacion relajada en los next.
8 GOTO 4850
9 rem ----- Jugada simulada nivel 1 -----
10 p= c (i): s (3)= sm (l)
20 for j= 0 to 3: a= ac (p, j): b= co (l) * ce (p, j): on (lo (a) - 3) goto 40, 50, 60, 70
30 goto 80
40 s (3)= s (3) - c * (s4 (p (a) ) - s4 (p (a) + b) ): goto 80
50 s (3)= s (3) - c * (s5 (p (a) ) - s5 (p (a) + b) ): goto 80
60 s (3)= s (3) - c * (s6 (p (a) ) - s6 (p (a) + b) ): goto 80
70 s (3)= s (3) - c * (s7 (p (a) ) - s7 (p (a) + b) )
80 next: return
99 rem ----- Jugada simulada niveles 1 y 2 -----
100 p= c (i): c (i)= p - 7: s (3)= sm (l)
110 for j= 0 to 3: a= ac (p, j): pm (l, j)= p (a): b= ce (p, j) * co (l): on (lo (a) - 3) goto 120, 130, 140, 150
115 goto 170
120 s (3)= s (3) - c * (s4 (p (a) ) - s4 (p (a) + b) ): goto 160
130 s (3)= s (3) - c * (s5 (p (a) ) - s5 (p (a) + b) ): goto 160
140 s (3)= s (3) - c * (s6 (p (a) ) - s6 (p (a) + b) ): goto 160
150 s (3)= s (3) - c * (s7 (p (a) ) - s7 (p (a) + b) )
160 p (a)= p (a) + b
170 next: sm (l + 1)= s (3): return
198 rem ----- Desmodificacion del juego -----
199 rem ----- cuando se remonta el arbol-----
200 c (i)= c (i) + 7: p= c (i): for j= 0 to 3: p (ac (p,j) )= pm (l, j): next: return
999 rem -------- Minimax 3 niveles ----------
1000 m= -1: s (0)= -20000: s (3)= 0: for i0= 0 to 6: i= o (i0): if c (i) < 0 then 1210
1010 l= 0: gosub 100: if s (3) > 5000 then gosub 200: m= i: goto 10200
1020 s (1)= 20000: for i1= 0 to 6: i= o (i1): if c (i) < 0 then 1170
1030 l= 1: gosub 100: if s (3) < -5000 then gosub 200: goto 1190
1040 s (2)= -20000: for i2= 0 to 6: i= o (i2): if c (i) < 0 then 1120
1050 l= 2: gosub 10: if s (3) > 5000 then s (2)= 20000: goto 1140
1100 if s (3) > s (2) then s (2)= s (3)
1120 next i2
1140 l= 1: i= o (i1): gosub 200
1150 if s (2) <= s (0) then 1190
1160 if s (2) < s (1) then s (1)= s (2)
1170 next i1
1180 if s (1) > s (0) then s (0)= s (1): m= o (i0)
1190 l= 0: i= o (i0): gosub 200
1210 next i0: if m <> -1 then return
1220 for i0= 0 to 6: if c (i0) >= 0 then m= i0: i0= 7
1230 next i0: return
4848 rem ----- Inicializacion definitiva
4850 defint a-z: dim s7 (2186), s6 (728), s5 (242), s4 (80), s (7), co (6), c(7), st (5, 7, 5), cd (2), ac (41, 3), p (36), ce (41, 3), lo (36), o (6), pm (5, 3), sm (5), a$ (1), tj$(41)
4860 a= 0: j= 0: p= 0: b= 0: l= 0: i= 0: i1= 0: i2= 0: i3= 0: lo= 0: i0= 0: c= 0
4999 rem st (i, j, k)
5000 for i1= 0 to 5: for i2= 4 to 7: for i3= 0 to 5: st (i1, i2, i3)= 10000: next: next: next
5020 for i= 1 to 5: for j= 1 to 5: st (i, 3, j)= 800: next: next
5040 for i= 1 to 5: st (0, 3, i)= 500: st (i, 3, 0)= 500: next
5060 for i= 2 to 5: st (i, 2, 1)= 200: st (1, 2, i)= 200: next
5080 for i= 2 to 5: for j= 2 to 5: st (i, 2, j)= 300: next: next
5120 for i= 2 to 5: st (0, 2, i)= 120: st (i, 2, 0)= 120: next: st (1, 2, 1)= 100
5399 rem ---Notacion de las 4000 lineas posibles ---
5400 cd (1)= 1: cd (2)= -1: for lo= 4 to 7: print tab (1);lo;:a= 0: i= 0: s (0)= 0: i1 (0)= 1: i2 (0)= 0: i3 (0)= 0
5410 if i = (lo - 1) then 5432
5420 i= i + 1: s(i)= 0: i1 (i)= i1 (i-1): i2 (i)= i2 (i-1): i3 (i)= i3 (i-1): co (i)= co (i-1): c (i)= 0: if i2 (i) = 0 then i1 (i)= i1 (i) + 1: goto 5410
5430 i3 (i)= i3 (i) + 1: goto 5410
5432 print tab (34); a;: if i2 (i) < 2 then p= 0: goto 5441
5434 p= st (i1 (i), i2 (i), i3 (i) ) * cd (co (i)): on (lo - 3) goto 5436, 5437, 5438, 5439
5436 s4 (a)= p: goto 5441
5437 s5 (a)= p: goto 5441
5438 s6 (a)= p: goto 5441
5439 s7 (a)= p
5441 for j= 3 to lo - 2: if c (j + 1) <> 1 then 5448
5442 if i2 (j) < 2 then j= lo: goto 5448
5443 i4= st (i1 (j), i2 (j), i3 (j) ) * cd (co (j) ) + p: on (lo - 3) goto 5444, 5445, 5446, 5447
5444 s4 (a)= i4: j= lo: goto 5448
5445 s5 (a)= i4: j= lo: goto 5448
5446 s6 (a)= i4: j= lo: goto 5448
5447 s7 (a)= i4: j= lo
5448 next
5450 if s (i) = 2 then 5520
5460 s (i)= s (i) + 1: a= a + 1: if i = 0 then i1 (0)= 0: i2 (0)= 1: i3 (0)= 0: co (0)= s(0): goto 5420
5470 i1 (i)= i1 (i-1): i2 (i)= i2 (i-1): i3 (i)= i3 (i-1): co (i)= co (i-1): c (i)= 0: if i2 (i) = 0 then co (i)= s (i): i2 (i)= 1: goto 5410
5480 if s (i) <> co (i) then 5510
5490 if i3 (i) <> 0 then 5510
5500 i2 (i)= i2 (i) + 1: goto 5410
5510 c (i)= 1: i1 (i)= i3 (i): i2 (i)= 1: i3 (i)= 0: co (i)= s (i): goto 5410
5520 if i = 0 then 5550
5530 i= i - 1
5540 goto 5450
5550 next lo
5999 rem ----- ce (i,j)
6000 for i=0 to 41 step 7: for j= 0 to 6: ce (i+j, 0)= 3 ^ j: ac (i+j, 2)=j: next:next: for i= 0 to 41: j= int (i/7): ce (i, 2)= 3 ^ j: ac (i, 0)= 7 + j: next
6010 data 1,1,1,1,1,1,1,1,3,3,3,3,3,3,1,3,9,9,9,9,9,1,3,9,27,27,27,27,1,3,9,27,81,81,81,1,3,9,27,81,243,243
6030 data 1,1,1,1,1,1,1,3,3,3,3,3,3,1,9,9,9,9,9,3,1,27,27,27,27,9,3,1,81,81,81,27,9,3,1,243,243,81,27,9,3,1
6250 for i= 1 to 3 step 2: for j= 0 to 41: read ce (j, i):next:next
6308 print
6309 rem ----- ac (i,j) -----
6310 data 18,19,20,21,22,23,24,17,18,19,20,21,22,23,16,17,18,19,20,21,22,15,16,17,18,19,20,21,14,15,16,17,18,19,20,13,14,15,16,17,18,19
6330 data 36,25,34,33,32,31,30,35,34,33,32,31,30,29,34,33,32,31,30,29,28,33,32,31,30,29,28,27,32,31,30,29,28,27,26,31,30,29,28,27,26,25
6340 for i= 1 to 3 step 2: for j= 0 to 41: read ac (j,i):next:next
6599 rem ----- lo (i) ---
6600 for i1= 0 to 6: lo (i1)= 6: next: for i1= 7 to 12: lo (i1)= 7: next
6610 data 3,3,3,4,5,6,6,5,4,3,3,3: for i1= 13 to 24: read a: lo (i1)= a: lo (i1 + 12)= a: next i1
6649 rem ----- o (i) -----
6650 data 3,2,4,1,5,0,6: for i1= 0 to 6: read o (i1): next
6699 rem ----- Notacion de las posiciones especiales
6700 s4 (31)= 500: s4 (37)= 500: s4 (62)= -500: s4 (74)= -500
6705 data 31,37,93,95,111,112,113,193,199: for i1= 1 to 9: read a: s5 (a)= 500: next
6710 data 62,74,143,155,186,187,222,223,224: for i1= 1 to 9: read a: s5 (a)= -500:next
6715 data 31,93,95,111,112,113,193,199,274,281,285,286,287,333,334,335,336,338,339,340,341,436,442,517,523,579,581,597,598,599,679,685: for i1= 1 to 32: read a: s6 (a)= 500: next
6720 s6 (37)= 620: s6 (279)= 620: s6 (280)= 800
6725 data 62,143,155,186,187,222,223,224,305,317,386,398,429,430,465,466,467,548,559,561,562,563,629,641,666,667,668,669,670,671,672,673:for i1= 1 to 32: read a: s6 (a)= -500: next
6730 s6 (74)= -620: s6 (558)= -620: s6 (560)= -800
6735 data 31,93,95,193,199,281,285,286,287,333,334,335,336,338,339,340,341,436,442,517,523,579,581,597,598,599,679,685,822,824,843,844,845,855,856,857,858,859,860,861,862,863,922,928,999,1001,1004,1005,1006
6740 data 1007,1010,1014,1015,1016,1017,1018,1019,1020,1021,1022,1023,1024,1025,1165,1171,1246,1252,1308,1310,1326,1327,1328,1408,1414,1489,1495,1551,1553,1569,1570,1571,1651,1657,1732,1739,1743,1744,1745,1791,1792,1793,1794,1796
6745 data 1797,1798,1799,1894,1900,1975,1981,2037,2039,2056,2057,2137,2143: for i1= 1 to 106: read a: s7 (a)= 500: next
6750 data 37,111,112,113,274,279,760,766,837,838,839,1000,1002,1008,1737: for i1= 1 to 15: read a: s7 (a)= 600: next
6755 data 280,840,841,842,1003,1009,1738: for i1= 1 to 7: read a: s7 (a)= 800: next
6760 data 62,143,155,186,187,305,317,386,398,429,430,465,466,467,559,561,562,563,629,641,666,667,668,669,670,671,672,673,791,803,872,884,915,916,951,952,953,1034,1046,1115,1127,1158,1159,1194,1195,1196,1277,1288,1290,1291
6765 data 1292,1358,1370,1395,1396,1397,1398,1399,1400,1401,1402,1601,1613,1645,1675,1677,1678,1679,1683,1684,1685,1686,1687,1688,1689,1690,1691,1763,1775,1844,1856,1887,1888,1923,1924,1925,1998,1999,2001
6770 data 2002,2003,2005,2007,2008,2009,2010,2011,2012,2013,2014,2015,2017,2019,2020,2021,2087,2099: for i1= 1 to 107: read a: s7 (a)= -500: next
6775 data 74,222,223,224,548,558,1287,1520,1532,1674,1676,2000,2004,2016: for i1= 1 to 14: read a: s7 (a)= -600: next
6780 data 560,1289,1680,1681,1682,2006,2118: for i1= 1 to 7: read a: s7 (a)= -800: next
6790 gosub 90000
6999 rem ----- Incializacion para cada partida -----
7000 for i1= 0 to 6: p (i1)= 0: next: for i1= 35 to 41: c (i1-35)= i1:next:dz= 2: nu= 0: for i1= 0 to 36: p (i1)= 0: next: for i1= 0 to 41: tj$ (i1)= " ": next: goto 9000
7999 rem ------ Mini apertura ------
8000 dz= 1: if co (0) = 2 then 8020
8010 m= 3: goto 10200
8020 if i = 3 then m= 2: goto 10200
8030 m= 3: goto 10200
8999 rem ----- Comienzo de la partida ----
9000 print "Comenzamos": input "Juego con las X o con las O"; a$: a$= upper$ (a$)
9020 if a$ = "X" then c= -1: m$= "X": j$= "O": co (0)= 2: co (1)= 1: co (2)= 2: co (3)= 1: co (4)= 2: gosub 16000: goto 10000
9030 if a$ = "O" then c=  1: m$= "O": j$= "X": c0 (0)= 1: co (1)= 2: co (2)= 1: co (3)= 2: co (4)= 1: goto 10100
9040 goto 9000
9999 rem ---- Entrada de la jugada ----
10000 input "Donde juegas"; ass: if ass < 1 or ass > 7 then 10000
10010 if c (ass - 1) < 0 then 10000
10020 i= ass - 1: tj$ (c(i))= j$: l= 3: sm (3)= 0: gosub 100: gosub 13000: if s (3) < -5000 then 15010
10100 print "Estoy pensando": on dz gosub 1000, 8000
10200 l= 4: i= m: tj$ (c (i) )= m$: gosub 100:  gosub 13000: print "Yo juego en la columna "; i + 1: if s (3) > 5000 then 15000
12210 goto 10000
12999 rem -----s/p jugadas -----
13000 nu= nu + 1: gosub 16000
13030 if nu <> 42 then return
13040 print "Partida nula"
13045 input "Para volver a jugar pulse (S) <RETURN>"; a$: gosub 15500: goto 7000
15000 print "He ganado": goto 13045
15010 print "He perdido": goto 13045
15500 if upper$ (a$) = "S" then return else end
15999 rem ----- Visualizacion del juego -----
16000 print: print "   1   2   3   4   5   6   7": for i1= 0 to 5: print " ! "; : for j1= 0 to 6: print tj$ (7 * i1 + j1); : print " ! "; : next: print: next: print " "; : print string$ (29, "-"): return
90000 rem Comprobacion
90010 print
90020 read a$
90030 if a$ <> "ENDDATA" then print "Error en DATA" : end
90040 return
90050 data "ENDDATA"
