v= sysvarptr
n = peek (v + 4) + 256 * peek (v + 5)
print "Numero de argumentos: "; n
for i= 1 to n
	print programarg$(i);
	if i  < n then print " ";
next
print
