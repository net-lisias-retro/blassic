REM points.bas

m= VAL (PROGRAMARG$ (1) )
IF m = 0 THEN m= 1 : goto standard_mode

h= val (programarg$ (2) )
if h = 0 then goto standard_mode

mode m, h
goto init

label standard_mode

MODE m

label init

d= SYSVARPTR
w= PEEK (d) + 256 * PEEK (d + 1)
h= PEEK (d + 2) + 256 * PEEK (d + 3)
randomize time
for y= 0 to h - 1
	for x= 0 to w - 1
		color rnd * 16
		PLOT x, y
		if inkey$ <> "" then stop
	next
NEXT
GET a$
END
