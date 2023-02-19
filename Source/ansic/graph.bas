rem graph.bas

rem ****************************
rem * Define the function here *
rem *      def fn y= f (x)     *
rem ****************************

rem def fn y (x)= (x - 1) * (x + 2) * x / 10

rem def fn y (x)= sin (x * 10)

rem def fn y (x)
rem	if x <> 0 then y= x * sin (1/x) else y= 0
rem fn end

def fn y (x)= sin (x*10) + .05 * sin (x*523) + 0.005 * sin (x * 5000) + 0.001 * sin (x * 27457) + 0.0005 * sin (x * 100000)

rem ***************
rem * Conversions *
rem ***************

def fn calculx (xx)= x1 + x21 * xx / xx2

def fn calculy (yy)= y1 + y21 * yy / yy2

def fn calculxx (x)= (x - x1) / x21 * xx2

def fn calculyy (y)= yy2 - ( (y - y1) / y21 * yy2)

goto init

rem ******************
rem * Initialisation *
rem ******************

label init

esc$= chr$ (27)

mode 10

x1= -pi: x2= pi
y1= -2 : y2= 2
x21= x2 - x1
y21= y2 - y1

d= sysvarptr
xx2= peek (d) + 256 * peek (d + 1) - 1
yy2= peek (d + 2) + 256 * peek (d + 3) -1

rem Initial values for zoom
rem inix= xx2 / 4
width= xx2 / 2
rem iniy= yy2 / 4
height= yy2 / 2

label showfunction

inix= (xx2 - width) / 2
iniy= (yy2 - height) / 2

label redraw
cls

rem Color 0, mode copy.
pen 0, , 0

rem ********
rem * Axis *
rem ********

xx= fn calculxx (0)
plot xx, 0 : draw xx, yy2

yy= fn calculyy (0)
plot 0, yy : draw xx2, yy

rem *****************
rem * Draw Function *
rem *****************

pen 1

xx= 0
x= fn calculx (xx)

y= fn y (x)

yy= fn calculyy (y)
move xx, yy

for xx= 1 to xx2
	x= fn calculx (xx)
	y= fn y (x)
	yy= fn calculyy (y)
	draw xx, yy
next

label tecla

rem Mode invert.
pen ,, 1

gosub recuadro

get a$ : a$= upper$ (a$)

rem color 15
gosub recuadro

rem Mode normal
pen ,,0

if a$ = "O" or a$ = "LEFT" then inix= inix - 1 : goto tecla
if a$ = "P" or a$ = "RIGHT" then inix= inix + 1 : goto tecla
if a$ = "I" then width= width + 1 : goto tecla
if a$ = "U" then width= width - 1 : goto tecla

if a$ = "Z" or a$ = "DOWN" then iniy= iniy + 1 : goto tecla
if a$ = "A" or a$ = "UP" then iniy= iniy - 1 : goto tecla
if a$ = "X" then height= height + 1 : goto tecla
if a$ = "S" then height= height - 1 : goto tecla

if a$ = "HOME" then inix= 0: goto tecla
if a$ = "END" then inix= xx2 - width: goto tecla

if a$ = "PAGEUP" then iniy= 0: goto tecla
if a$ = "PAGEDOWN" then iniy= yy2 - height: goto tecla

if a$ = "Q" or a$ = esc$ then goto endprogram

if a$ = "R" then goto redraw

if a$ = "V" then pen 0: locate 1, 1: print "X1= "; x1; " X2= "; x2; " Y1= "; y1; " Y2= "; y2 : goto tecla

if a$ = "CLICK" then gosub mousedraw: goto tecla

if a$ <> " " then goto tecla

xs1= fn calculx (inix)
xs2= fn calculx (inix + width)

ys1= fn calculy (yy2 - (iniy + height) )
ys2= fn calculy (yy2 - iniy)

if xs1 = xs2 or ys1 = ys2 then pen 0: locate 1, 1: print "**TOO CLOSE**" : goto tecla

x1= xs1: x2= xs2: y1= ys1: y2= ys2
x21= x2 - x1
y21= y2 - y1

goto showfunction

label mousedraw

pen ,,1

oinix= inix: oiniy= iniy: oheight= height: owidth= width

inix= xmouse: iniy= ymouse: height= 0: width= 0
gosub recuadro

repeat
	nw= xmouse - inix: nh= ymouse - iniy
	if width <> nw or height <> nh then gosub recuadro : width= nw: height= nh: gosub recuadro
	a$= inkey$
until a$ <> ""

gosub recuadro

if a$ = esc$ then inix= oinix: iniy= oiniy: width= owidth: height= oheight

if width < 0 then inix= inix + width: width= -width

if height < 0 then iniy= iniy + height: height= - height

pen ,,0

return

label recuadro
plot inix, iniy to inix, iniy + height to inix + width, iniy + height to inix + width, iniy to inix, iniy
return

label endprogram

end
