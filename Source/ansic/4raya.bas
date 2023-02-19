1 GOTO 4850
4850 DIM s7 (2186), s6 (728), s5 (242), s4 (80), s (7), co (6), c(7), st (5, 7, 5), cd (2), ac (41, 3), p (36), ce (41, 3), lo (36), o(6), pm (5, 3), sm (5), a$ (1), tj$(41)
4860 a= 0: j= 0: p= 0: b= 0: l= 0: i= 0: i1= 0: i2= 0: i3= 0: lo= 0: i0= 0: c= 0
4999 rem st (i, j, k)
5000 for i1= 0 to 5: for i2= 4 to 7: for i3= 0 to 5: st (i1, i2, i3)= 10000: next: next: next
5020 for i= 1 to 5: for j= 1 to 5: st (i, 3, j)= 800: next: next
5040 for i= 1 to 5: st (0, 3, i)= 500: st (i, 3, 0)= 500; next
5060 for i= 2 to 5: st (i, 2, 1)= 200: st (1, 2, i)= 200: next
5080 for i= 2 to 5: for j= 2 to 5: st (i, 2, j)= 300: next: next
5120 for i= 2 to 5: st (0, 2, i)= 120: st (i, 2, 0)= 120: next: st (1, 2, 1)= 100
5399 rem ---Notacion de las 4000 lineas posibles ---
5400 cd (1)= 1: cd (2)= -1: for lo= 4 to 7: print tab (1);lo;:a= 0: i= 0: s(0)= 0: i1(0)= 1: i2 (0)= 0: i3 (0)= 0
5410 if i = (lo - 1) then 5432
5420 i= i + 1: s(i)= 0: i1 (i)= i1 (i-1): i2 (i)= i2 (i-1): i3 (i)= i3 (i-1): co (i)= co (i-1): c (i)= 0: if i2 (i) = 0 then i1 (i)= i1 (i) + 1: goto 5410
5430 i3 (i)= i3 (i) + 1: goto 5410
5432 print tab (34); a;: if i2 (i) < 2 then p= 0: goto 5441
5434 p= st (i1 (i), i2 (i), i3 (i) ) * cd (co (i)): on (lo - 3) goto 5436, 5437, 5438, 5439
5436 s4 (a)= p: goto 5441
5437 s5 (a)= p: goto 5441
5438 s6 (a)= p: goto 5441
5439 s7 (a)= p
5441 for j= 3 to lo-2: if c(j+1) <> 1 then 5448
5442 if i2 (j) < 2 then j= lo: goto 5448
5443 i4= st (i1 (j), i2 (j), i3 (j) ) * cd (co (j) ) + p: on (lo - 3) goto 5444, 5445, 5446, 5447
5444 s4 (a)= i4: j= lo: goto 5448
5445 s5 (a)= i4: j= lo: goto 5448
5446 s6 (a)= i4: j= lo: goto 5448
5447 s7 (a)= i1: j= lo
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
