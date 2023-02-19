rem mouse.bas
rem Draw with the mouse.

def fn waitkey$
	get waitkey$
fn end

esc$= chr$ (27)

mode 10

while (let tec$= fn waitkey$) <> esc$
	if tec$ <> "CLICK" then goto loopagain

	x= xmouse: y= ymouse
	plot x, y
	while inkey$ = ""
		x1= xmouse: y1= ymouse
		if x1 <> x or y1 <> y then draw x1, y1: x= x1: y= y1
	wend

	label loopagain
wend

end
