rem randomw.bas

open "random.dat" for random as #1 len=14
field #1, 2 as i$, 4 as s$, 8 as d$

read n
for i= 1 to n
	read i, s, d
	print i, s, d
	lset i$= mki$ (i)
	lset s$= mks$ (s)
	lset d$= mkd$ (d)
	put #1, i
next

close 1

end

data 9

data 1, 1.111111, 1.11111111111111111
data 2, 2.222222, 2.22222222222222222
data 3, 3.333333, 3.33333333333333333
data 4, 4.444444, 4.44444444444444444
data 5, 5.555555, 5.55555555555555555
data 6, 6.666666, 6.66666666666666666
data 7, 7.777777, 7.77777777777777777
data 8, 8.888888, 8.88888888888888888
data 9, 9.999999, 9.99999999999999999

rem End of randomw.bas
