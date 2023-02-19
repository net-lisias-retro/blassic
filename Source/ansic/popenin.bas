rem popenin.bas

if osfamily$ = "windows" then co$= "dir /b" else co$= "ls"

popen co$ for input as #1
print "Opened"

n= 0
maxn= 5000
dim a$ (maxn)

while not eof (1) and n < maxn
	n= n + 1
	line input #1, l$
	a$ (n)= l$
wend
close 1
print "Closed"


for i= 1 to n
	print a$ (i)
next

end
