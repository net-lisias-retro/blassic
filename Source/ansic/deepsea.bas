rem		*****************************
rem		**  D E E P S E A . B A S  **
rem		*****************************

rem	*********************************************************
rem	**  A simple game demostrating Blassic's capabilites.  **
rem 	**                                                     **
rem	**        (C) 2002 Julian Albo "NotFound"              **
rem	**                                                     **
rem	**  Distributable under the terms of the GPL License.  **
rem	*********************************************************

def int a-z

randomize time

color_sky= 11
color_sea= 1

sealevel= 7
boatlevel= sealevel - 1
minysub= sealevel + 1

maxbomb= 5
dim xbomb (maxbomb), ybomb (maxbomb), counter (maxbomb)
nbomb= 0
inicount= 15

maxboat= 6
dim xboat (maxboat), incboat (maxboat), countboat (maxboat)
nboat= 0
inicountboat= 10

maxglub= 20
dim xglub (maxglub), countglub (maxglub)
inicountglub= 104

maxcharge= 30
dim xcharge (maxcharge), ycharge (maxcharge), countcharge (maxcharge)
inicountcharge= 10

mode 5

synchronize 1

gosub clearscreen

synchronize

gosub defchars

d= sysvarptr
charset= peek (d+20) + 256 * (peek (d+21) + 256 * (peek (d+22) + 256 * peek (d+23) ) )

g$= "DEEP": posx= 5: posy= sealevel + 2: gosub rotulo

pause 500

g$= "SEA": posx= 9: posy= sealevel + 10: gosub rotulo

pause 750

xsub= 20: ysub= 20

pen 6: paper color_sea
locate ysub, xsub: print sub$;
synchronize

pause 500

gosub clearscreen

pen 6: paper color_sea

gosub printpoints

repeat
	locate ysub, xsub: print sub$;
	synchronize
	pause 10

	a$= upper$ (inkey$)
	if a$ = "P" then get a$: a$= upper$ (a$)
	oxsub= xsub: oysub= ysub
	if a$ = "LEFT" then xsub= xsub - 1
	if a$ = "RIGHT" then xsub= xsub + 1
	if a$ = "UP" then ysub= ysub - 1
	if a$ = "DOWN" then ysub= ysub + 1
	xsub= max (min (xsub, 38), 1)
	ysub= max (min (ysub, 24), minysub)

	locate oysub, oxsub: print clearsub$;
	gosub moveall
	if a$ = " " then gosub firebomb

until a$ = "Q"

end

label moveall

gosub moveboats

gosub showglubs

pen 6, 0: paper color_sea

for i= 1 to maxbomb
	if xbomb (i) <> 0 then locate ybomb (i), xbomb (i): print " ";
next

for i= 1 to maxcharge
	if xcharge (i) <> 0 then locate ycharge (i), xcharge (i): print " ";
next

pen , 1

gosub movebombs

gosub movecharges

pen , 0

gosub printpoints

return

label movebombs

for i= 1 to maxbomb

	if xbomb (i) = 0 then goto nextbomb

	counter (i)= counter (i) - 1
	if counter (i) > 0 then goto printbomb
	counter (i)= inicount

	ybomb (i)= ybomb (i) - 1
	if ybomb (i) >= sealevel then goto printbomb

	killpos= xbomb (i)	
	xbomb (i)= 0: nbomb= nbomb - 1

	killboat= 0
	for j= 1 to maxboat
		if xboat (j) = 0 then goto nextbombboat
		touch= killpos - xboat (j)
		if touch >= 0 and touch <= 2 then killboat= j: j= maxboat
		
		label nextbombboat
	next
	if killboat = 0 then goto nextbomb

	newglub= 0
	for j= 1 to maxglub
		if xglub (j) = 0 then newglub= j: j= maxglub
	next
	if newglub <> 0 then xglub (newglub)= xboat (killboat): countglub (newglub)= inicountglub

	xboat (killboat)= 0: nboat= nboat - 1
	points= points + 10

	goto nextbomb

	label printbomb
	locate ybomb (i), xbomb (i): print bomb$;

	label nextbomb
next

return

label movecharges

killed= 0

for i= 1 to maxcharge
	if xcharge (i) = 0 then goto nextcharge
	countcharge (i)= countcharge (i) - 1
	if countcharge (i) > 0 then goto nomovecharge
	countcharge (i)= inicountcharge
	ycharge (i)= ycharge (i) + 1
	if ycharge (i) >= 25 then xcharge (i)= 0 : goto nextcharge

	label nomovecharge

	locate ycharge (i), xcharge (i): print charge$;

	dist= xcharge (i) - xsub
	if ycharge (i) = ysub and dist >= 0 and dist < 3 then killed= 1: locate ycharge (i), xcharge (i) - 2: print "(( ))";

	label nextcharge
next

if killed then goto gameover

return

label firecharge

if xfire < 1 or xfire > 40 then return

x= 0
for i= 1 to maxcharge
	if xcharge (i) = 0 then x= i: i= maxcharge
next
if x = 0 then return

xcharge (x)= xfire
ycharge (x)= sealevel
countcharge (x)= inicountcharge
pen 6: paper color_sea
locate ycharge (x), xcharge (x): print charge$;

return

label showglubs

for i= 1 to maxglub
	if xglub (i) = 0 then goto nextglub
	countglub (i)= countglub (i) - 1
	if countglub (i) = 0 then xglub (i)= 0: goto nextglub
	g= countglub (i) / 15
	locate boatlevel, xglub (i) : print glub$ (g);

	label nextglub
next

return

label moveboats

pen 4, 0: paper color_sky

locate boatlevel, 1: print spc (40);

pen , 1

for i= 1 to maxboat
	if xboat (i) = 0 then goto nextboat
	countboat (i)= countboat (i) - 1
	if countboat (i) > 0 then goto printboat

	countboat (i)= inicountboat
	xboat (i)= xboat (i) + incboat (i)
	if (incboat (i) < 0 and xboat (i) < 1) or (incboat (i) > 0 and xboat (i) > 37) then xboat (i)= 0: nboat= nboat - 1: goto nextboat

	label printboat

	locate boatlevel, xboat (i): print boat$;

	label fireboat

	if rnd > 0.015 then goto nextboat
	xfire= xboat (i)
	if incboat (i) > 0 then xfire= xfire + 4: else xfire= xfire - 2
	gosub firecharge

	label nextboat
next

if nboat >= maxboat or rnd > 0.015 then goto endmoveboats

posboat= 0
for i= 1 to maxboat
	if xboat (i) = 0 then posboat= i: i= maxboat
next
if posboat= 0 then goto endmoveboats

nboat= nboat + 1

if rnd > 0.5 then goto boatfromright

xboat (posboat)= 1
incboat (posboat)= 1

goto finishboat

label boatfromright

xboat (posboat)= 37
incboat (posboat)= -1

label finishboat

coutboat (posboat)= inicountboat

locate boatlevel, xboat (posboat): print boat$;

label endmoveboats

return

label firebomb

	if nbomb >= maxbomb then return

	posbomb= 0
	for i= 1 to maxbomb
		if xbomb (i) = 0 then posbomb= i: i= maxbomb
	next
	if posbomb = 0 then return

	nbomb= nbomb + 1
	ybomb (posbomb)= ysub - 1
	xbomb (posbomb)= xsub + 2
	counter (posbomb)= inicount
	locate ybomb (posbomb), xbomb (posbomb): print bomb$;

return

label printpoints

locate 25, 1: print using "Points: ######", points;

return

label gameover

synchronize

pause 1000

posx= 5
g$= "GAME": posy= sealevel + 2: gosub rotulo

pause 500

g$= "OVER": posy= sealevel + 10: gosub rotulo

while inkey$ <> "" : wend

get a$

end

label rotulo

l= len (g$)
pen 0, 0
for i= 0 to 7
	for j= 1 to l
		posj= (j - 1) * 8
		c= asc (mid$ (g$, j, 1) )
		n= peek (charset + c * 8 + i)
		m= 128
		for k= 0 to 7
			locate posy + i, posx + k + posj
			if n and m then print black$;
			m= m / 2
		next
	next
	synchronize
	pause 100
next

return

end

label clearscreen

paper color_sea
cls

paper color_sky
print space$ (40 * (sealevel - 1) )

return

label defchars

symbol after 233

symbol 255, 0, 0, 0, 31, 127, 255, 127, 31
symbol 254, 248, 248, 248, 255, 255, 255, 255, 255
symbol 253, 0, 0, 0, 248, 254, 255, 254, 248
sub$= chr$ (255) + chr$ (254) + chr$ (253)
clearsub$= space$ (len (sub$) )

symbol 252, 0, 0, 0, 0, 255, 255, 127, 63
symbol 251, 4, 4, 255, 255, 255, 255, 255, 255
symbol 250, 0, 0, 0, 0, 255, 252, 240, 192
boat$= chr$ (252) + chr$ (251) + chr$ (250)
clearboat$= space$ (len (boat$) )

symbol 249, 16, 56, 56, 56, 56, 56, 124, 214
bomb$= chr$ (249)

symbol 248, 16, 56, 124, 254, 127, 62, 28, 8
charge$= chr$ (248)

symbol 247, 255, 255, 255, 255, 255, 255, 255, 255
black$= chr$ (247)

symbol 246, 0, 224, 124, 127, 63, 63, 31, 31
symbol 245, 0, 0, 0, 128, 240, 254, 255, 255
symbol 244, 0, 0, 0, 0, 0, 0, 192, 248

symbol 243, 0, 0, 224, 124, 127, 63, 63, 31
symbol 242, 0, 0, 0, 0, 128, 240, 254, 255
symbol 241, 0, 0, 0, 0, 0, 0, 0, 192

symbol 240, 0, 0, 0, 224, 124, 127, 63, 63
symbol 239, 0, 0, 0, 0, 0, 128, 240, 254

symbol 238, 0, 0, 0, 0, 224, 124, 127, 63
symbol 237, 0, 0, 0, 0, 0, 0, 128, 240

symbol 236, 0, 0, 0, 0, 0, 224, 124, 127
symbol 235, 0, 0, 0, 0, 0, 0, 0, 128

symbol 234, 0, 0, 0, 0, 0, 0, 224, 124

symbol 233, 0, 0, 0, 0, 0, 0, 0, 224

dim glub$ (6)
glub$ (6)= chr$ (246) + chr$ (245) + chr$ (244)
glub$ (5)= chr$ (243) + chr$ (242) + chr$ (241)
glub$ (4)= chr$ (240) + chr$ (239)
glub$ (3)= chr$ (238) + chr$ (237)
glub$ (2)= chr$ (236) + chr$ (235)
glub$ (1)= chr$ (234)
glub$ (0)= chr$ (233)

return
