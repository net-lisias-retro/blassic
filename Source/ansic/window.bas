rem window.bas
mode 10
for i= 0 to 15
	window #i, 1 + i, 80 - i, 1 + i, 32 - i
	paper #i, i
	rem if i = 0 then pen 15 else pen 0
	pen #i, 15 - i
	cls #i
	print #i, "This is window "; i;
	pause 100
next

for i= 0 to 15
	print #i, " Again in window "; i;
	pause 50
next

pause 500

for i= 0 to 15
	locate #i, 32 - 2 * i, 1
	print #i, "One more time in window "; i;
	pause 50
next

get a$

for i= 14 to 0 step -1
	cls #i
	pause 100
next

get a$
