rem textcolor.bas

cls

label sigue

read name$
if name$= "***" then end
read num

if num = 7 then paper 0: else paper 7
pen num
print "Color "; num; ": "; name$

goto sigue

data "Negro", 0
data "Azul", 1
data "Verde", 2
data "Cyan", 3
data "Rojo", 4
data "Magenta", 5
data "Marron", 6
data "Gris claro", 7
data "Gris oscuro", 8
data "Azul claro", 9
data "Verde claro", 10
data "Cyan claro", 11
data "rojo claro", 12
data "violeta", 13
data "amarillo", 14
data "Blanco", 15
data "***"
