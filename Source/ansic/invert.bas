rem invert.bas

d= sysvarptr

rem charset= peek (d + 20) + 256 * (peek (d + 21) + 256 * (peek (d + 22) + 256 * peek (d+ 23) ) )
charset= peek32 (d + 20)

mode 10

gosub invert

list

print

gosub invert

list

get a$

end

label invert

for i= charset + asc ("!") * 8 to charset + 256 * 8 - 1
	poke i, not peek (i)
next

return