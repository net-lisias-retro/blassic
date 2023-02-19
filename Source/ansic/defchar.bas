rem defchar.bas

gosub define_chars

dim chardata (8, 8)
dim charbyte (8)
esc$= chr$ (27)
del$= chr$ (127)
sym= 246

mode 10

xini= 10
yini= 10

locate yini, xini: print chr$ (250); string$ (8, 252); chr$ (249)
for i= 1 to 8
	locate yini + i, xini: print chr$ (251);
	locate yini + i, xini + 9: print chr$ (251);
next
locate yini + 9, xini: print chr$ (248); string$ (8, 252); chr$ (247)


x= 1: y= 1

if chardata (y, x) = 0 then c= 253 else c= 254

locate yini + y, xini+ x
print chr$ (c);

repeat
	get a$
	a$ = upper$ (a$)
	ox= x: oy= y
	if a$ = "P" or a$ = "RIGHT" then x= x + 1
	if a$ = "O" or a$ = "LEFT" then x= x - 1
	if a$ = "A" or a$ = "UP" then y= y - 1
	if a$ = "Z" or a$ = "DOWN" then y= y + 1
	if a$ = " " then chardata (y, x)= 1: goto redraw
	if a$ = "E" or a$ = del$ then chardata (y, x)= 0: goto redraw
	if a$ = "D" then gosub showdef : goto sigue
	if x < 1 then x= 1
	if x > 8 then x= 8
	if y < 1 then y= 1
	if y > 8 then y= 8
	if ox = x and oy = y then goto sigue
	if chardata (oy, ox) = 0 then c= 32 else c= 255
	locate yini + oy, xini + ox
	print chr$ (c);

	label redraw

	if chardata (y, x) = 0 then c= 253 else c= 254
	locate yini + y, xini+ x
	print chr$ (c);
	gosub redef
	locate yini + 2, xini + 12: print chr$ (sym)
	
	label sigue
	
until a$ = "Q" or a$ = esc$

end

label redef

for i= 1 to 8
	b= 1
	c= 0
	for j= 8 to 1 step -1
		c= c + chardata (i, j) * b
		b= b * 2
	next
	charbyte (i)= c
next

symbol sym, charbyte (1), charbyte (2), charbyte (3), charbyte (4), charbyte (5), charbyte (6), charbyte (7), charbyte (8)

return

label showdef

locate yini + 12, 1
print spc (60)
locate yini + 12, 1

print "symbol n, ";
for i= 1 to 8
	print charbyte (i);
	if (i < 8) then print ", ";
next
print
return

label define_chars

read c

if c = 999 then return

gosub read_char

goto define_chars

label read_char

for i= 1 to 8
	read a$
	a (i)= val ("&X" + a$)
next

symbol c, a (1), a (2), a (3), a (4), a (5), a (6), a (7), a (8)

return

data 255

data 11111111
data 11111111
data 11111111
data 11111111
data 11111111
data 11111111
data 11111111
data 11111111

data 254

data 11111111
data 11101111
data 11101111
data 10000011
data 11101111
data 11101111
data 11111111
data 11111111

data 253

data 00000000
data 00010000
data 00010000
data 01111100
data 00010000
data 00010000
data 00000000
data 00000000

data 252

data 00000000
data 00000000
data 00000000
data 11111111
data 11111111
data 00000000
data 00000000
data 00000000

data 251

data 00011000
data 00011000
data 00011000
data 00011000
data 00011000
data 00011000
data 00011000
data 00011000

data 250

data 00000000
data 00000000
data 00000000
data 00011111
data 00011111
data 00011000
data 00011000
data 00011000

data 249

data 00000000
data 00000000
data 00000000
data 11111000
data 11111000
data 00011000
data 00011000
data 00011000

data 248

data 00011000
data 00011000
data 00011000
data 00011111
data 00011111
data 00000000
data 00000000
data 00000000

data 247

data 00011000
data 00011000
data 00011000
data 11111000
data 11111000
data 00000000
data 00000000
data 00000000

data 999

rem End of defchar.bas
