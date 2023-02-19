
esc$= chr$ (27)
a= 0
iniinca= 0.04
mula= 1.2
inca= iniinca
a1= 0
a2= pi / 2
a3= pi
a4= 3 * pi / 2

inix= 250
iniy= 175
orgx= inix
orgy= iniy
incx= 2
incy= 2
inil= 125
l= inil
incl= 2

inicol= 15
inilin= 14

mode 500, 350
paper 4
cls

pulsa$= "Pulsa cualquier tecla para comenzar"

locate inilin,      inicol: print "ESC o Q ---> Terminar"
locate inilin +  2, inicol: print "Flechas ---> Mover"
locate inilin +  4, inicol: print "I       ---> Cambiar direccion"
locate inilin +  6, inicol: print "+ y -   ---> Cambiar velocidad"
locate inilin +  8, inicol: print "M y N   ---> Cambiar longitud del lado"
locate inilin + 10, inicol: print "P       ---> Pausa"
locate inilin + 12, inicol: print "Inicio  ---> Posicion inicial"

linpulsa= inilin + 16

pen , 1, 4

gosub cualquiera

get a$

gosub nocualquiera

pulsa$= "Pulsa cualquier tecla para continuar"

gosub calcula
gosub dibuja
synchronize 1
repeat
	pause 5
	gosub dibuja
	a$= upper$ (inkey$)
	if a$ = "Q" or a$ = esc$ then termina= 1
	if a$ = "LEFT" then orgx= orgx - incx
	if a$ = "RIGHT" then orgx= orgx + incx
	if a$ = "UP" then orgy= orgy - incy
	if a$ = "DOWN" then orgy= orgy + incy
	if a$ = "HOME" then orgx= inix: orgy= iniy: a= 0: inca= iniinca: l= inil
	if a$ = "I" then inca= -inca
	if a$ = "+" then inca= inca * mula
	if a$ = "-" then inca= inca / mula
	if a$ = "M" then l= l + incl
	if a$ = "N" then l= l - incl
	a= a + inca
	if a > 2 * pi then a= a - 2 * pi
	if a < -2 * pi then a= a + 2 * pi
	gosub calcula
	gosub dibuja
	synchronize
	if a$ = "P" then gosub pausa
until termina

end

label pausa

synchronize 0
gosub cualquiera
get b$
gosub nocualquiera
synchronize 1

return

label cualquiera

locate linpulsa, inicol

for i= 1 to len (pulsa$)
	print mid$ (pulsa$, i, 1);
	pause 10
next

return

label nocualquiera

for i= len (pulsa$) to 1 step -1
	locate linpulsa, inicol - 1 + i
	print mid$ (pulsa$, i, 1)
	pause 10
next

return

label calcula

x1= orgx + l * cos (a + a1)
y1= orgy - l * sin (a + a1)
x2= orgx + l * cos (a + a2)
y2= orgy - l * sin (a + a2)
x3= orgx + l * cos (a + a3)
y3= orgy - l * sin (a + a3)
x4= orgx + l * cos (a + a4)
y4= orgy - l * sin (a + a4)
return

label dibuja

move x1, y1
draw x2, y2
draw x3, y3
draw x4, y4
draw x1, y1
return
