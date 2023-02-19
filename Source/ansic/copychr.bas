mode 10
pen 1
paper 4
print "Hello World"
draw 100, 100
pen 0
paper 15
for i= 1 to 12
	locate 1, i
	a$= copychr$ (#0)
	locate 4 + i, 1
	print "<" + a$ + ">: "; asc (a$)
next
get a$
