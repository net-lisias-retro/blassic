rem Some uses of the CIRCLE command.

mode 10

circle (300, 200), 100

mask &x10101010

circle (300, 200), 100, , , , .5
circle (300, 200), 100, , , , 2

rem circle (300, 200),  90, , pi/4, 7 * pi / 4, .5

mask &ff

graphics pen 15
mover 0,0, , 1 : rem xor mode

synchronize 1

pi2= pi * 2
ninc= pi / 18
r1= pi / 2
p= 100

repeat

	for i= 1 to 80
		gosub do_draw
		if inkey$ <> "" then goto the_end
	next

	for i= 79 to 2 step -1
		gosub do_draw
		if inkey$ <> "" then goto the_end
	next

until 0

label the_end

synchronize 0

end

label do_draw

a= 80 / i
b= (81 - i) / 80

gosub do_circles

synchronize
pause p

gosub do_circles

gosub inc_n

return

label do_circles

mask &fc

circle (300, 200),  90, , n + pi/4, n+ 7 * pi / 4, .5

mask &ff

circle (300, 390), 80, ,   , , a
circle (330, 390), 80, , r1, , a

circle (500, 290), 81, ,   , , b
circle (500, 320), 81, , r1, , b

return

label inc_n

n= n + ninc
if n > pi2 then n= n - pi2

return

rem End of circles.bas
