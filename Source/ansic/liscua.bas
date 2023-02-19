rem liscua.bas

print

for i= 1 to 10 step 2
	j= i + 1
	print using "El cuadrado de ## es ###, "; i; i * i; j; j * j
next

print

for i= 1 to 10
	print using "Raiz de ## es #.######"; i; sqr (i)
next
