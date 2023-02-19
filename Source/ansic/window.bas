rem window.bas
mode 10
for i= 0 to 15
	window #i, 1 + i, 80 - i, 1 + i, 31 - i
	paper #i, i
	rem if i = 0 then pen 15 else pen 0
	pen #i, 15 - i
	cls #i
	print #i, "This is window "; i;
	pause 100
next
get a$
