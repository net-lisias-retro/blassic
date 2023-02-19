rem path.bas

if osfamily$ = "windows" then sep$= ";" else sep$= ":"

path$= environ$ ("PATH")

p= instr (path$, sep$)

while p > 0
	print left$ (path$, p - 1)
	path$= mid$ (path$, p + 1)
	p= instr (path$, sep$)
wend

print path$

end
