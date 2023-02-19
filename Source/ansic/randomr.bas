10 rem randomr.bas
20 rem
30 open "random.dat" for random as #1 len=14
40 field #1, 2 as i$, 4 as s$, 8 as d$
50 rem
60 for i= 1 to 9
70	get #1
80	print cvi (i$), cvs (s$), cvd (d$)
90 next
100 rem
110 close 1
120 rem
130 end
140 rem
150 rem End of randomr.bas
