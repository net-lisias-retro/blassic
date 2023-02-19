rem shellsort.bas

rem def int a, d, g, i-j, n

n= val (programarg$ (1) )
if n = 0 then n= 500
dim a (n)

for i= 1 to n
	a (i)= i
next

randomize time
for i= 1 to 2 * n
	i1= rnd * n + 1
	i2= rnd * n + 1
	swap a (i1), a (i2)
next

gosub print_a

print "Sorting..."

rem Rutina de ordenacion tomada de un articulo de Andre Warusfel.
rem El Ordenador Personal Num 34, Marzo 1985.

dim g (n/2), d (n/2)

p= 1: g(1)= 1: d (1)= n

label l50: g= g (p): d= d (p): p= p - 1
label l60: i= g: j= d: x= (a (g) + a (d) ) /2
label l70: if a (i) < x then let i= i + 1: goto l70
label l80: if a (j) > x then let j= j - 1: goto l80
if i <=j then swap a (i), a (j): i= i + 1: j= j - 1
if i <= j then goto l70
if i < d then let p= p + 1: g (p)= i: d (p)= d
if g < j then let d= j: goto l60
if p <> 0 then goto l50

gosub print_a

failed= 0
for i= 1 to n - 1
	if a (i) >= a (i + 1) then failed= 1
next

if failed then print "********* ERROR *********" : exit 1

end

label print_a

for i= 1 to n
	print a (i); " ";
next
print

return
