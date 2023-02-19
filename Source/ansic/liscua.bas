rem liscua.bas

print

for i= 1 to 10 step 2
	j= i + 1
	print using "El cuadrado de ## es ###, "; i; i * i; j; j * j
next

print

a$= "Raiz de ### es ##.######
for i= 1 to 10
	print using a$; i; sqr (i)
next
for i= 100 to 900 step 100
	print using a$; i; sqr (i)
next
