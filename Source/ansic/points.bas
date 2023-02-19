REM points.bas

m= VAL (PROGRAMARG$ (1) )
IF m = 0 THEN m= 1

h= val (programarg$ (2) )
if h = 0 then mode m : else mode m, h

label init

d= SYSVARPTR
w= PEEK (d) + 256 * PEEK (d + 1)
h= PEEK (d + 2) + 256 * PEEK (d + 3)
randomize time
for i= 1 to w * h / 10
	color rnd * 16
	x= RND * w
	y= RND * h
	PLOT x, y
NEXT i
GET a$
END
