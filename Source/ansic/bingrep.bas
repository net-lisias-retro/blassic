rem bingrep
rem Find strings in binary files
rem (C) 2002 Julian Albo

open error as #1

find$= programarg$ (1)

if find$ = "" or programarg$ (2) = "" then print #1, "Usage: blassic bingrep string-to-find file(s)": exit 2

found= 0
param= 2

on error goto notopen

while programarg$ (param) <> ""

	filename$= programarg$ (param)
	load filename$, file$

	rem pos= instr (file$, find$)
	pos= 0

	rem while pos > 0
	while (let pos= instr (pos + 1, file$, find$) ) > 0
		found= 1
		print filename$; " AT "; pos 
		rem pos= instr (pos + 1, file$, find$)
	wend

	label nextfile
	param= param + 1
wend

if found = 0 then exit 1
exit 0

label notopen
if err = 30 then print #1, filename$; " NOT FOUND": resume nextfile

print #1, "Error "; ERR; " en linea "; ERL
exit 3

rem End of bingrep
