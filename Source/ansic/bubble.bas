rem bubble.bas

rem def int a, i-j, n

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

for i= n to 1 step -1
	sorted= 1
	for j= 2 to i
		if a (j - 1) > a (j) then swap a (j - 1), a (j) : sorted= false
	next
	if sorted then i= 1
next

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
