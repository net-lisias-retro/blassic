rem gradient.bas

mode 400, 256

for i= 0 to 255
	c= 255 - i
	ink i, c, c, c
	graphics pen i
	line (0, i) - (799, i)
next

open error as #1

lant= 0
repeat
	l= ymouse
	c= test (0, l)
	if l <> lant then print #1, l, c: lant= l
until inkey$ <> ""
