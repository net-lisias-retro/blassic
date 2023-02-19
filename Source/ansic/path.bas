rem path.bas

if osfamily$ = "windows" then sep$= ";" else sep$= ":"

path$= environ$ ("PATH")

pos= instr (path$, sep$)

while pos > 0
	print left$ (path$, pos - 1)
	path$= mid$ (path$, pos + 1)
	pos= instr (path$, sep$)
wend

print path$

end
