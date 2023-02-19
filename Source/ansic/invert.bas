rem invert.bas

d= sysvarptr
charset= peek32 (d + 20)

def fn mirror (a)
	local i
	mirror= 0
	for i= 1 to 8
		mirror= mirror * 2
		if a and 1 then mirror= mirror or 1
		a= a / 2
	next
fn end

mode 10

print "Inverse": print

gosub invert
list
print
gosub invert
get a$

print "Mirror": print

gosub mirrorize
list
print
gosub mirrorize
get a$

print "Inverse & mirror": print

gosub invert
gosub mirrorize
list
print
gosub invert
gosub mirrorize
get a$

end

label invert

for i= charset + asc ("!") * 8 to charset + 256 * 8 - 1
	poke i, not peek (i)
next

return

label mirrorize

for i= charset + asc ("!") * 8 to charset + 256 * 8 - 1
	poke i, fn mirror (peek (i) )
next

return
