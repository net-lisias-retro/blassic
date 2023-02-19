REM
REM htget.bas
REM Download a page from an http server.
REM

debug= 0

url$= programarg$ (1)

if lower$ (url$) = "-d" then debug= 1: url$= programarg$ (2)

open error as #2

if url$ = "" then url$= "http://www.xente.mundo-r.com/notfound/blassic/"

if instr (url$, "http://") = 1 then url$= mid$ (url$, 8)

pos= instr (url$, "/")

if pos <> 0 then host$= left$ (url$, pos - 1) : page$= mid$ (url$, pos) else host$= url$: page$= ""

if page$ = "" then page$= "/"

if debug then print #2, url$; ": "; host$; ", "; page$

on error goto no_host

socket host$, 80 as #1

on error goto 0

print #1, "GET "; page$; " HTTP/1.1"
print #1, "host: "; host$
print #1, "Connection: close"
print #1

rem Skip http header

isChunked= 0
a$= "*"
l= 0
while not eof (1) and a$ <> "" and a$ <> chr$ (13)
	line input #1, a$
	if debug then print #2, a$
	if upper$ (left$ (a$, 18) ) = "TRANSFER-ENCODING:" then isChunked= not 0
	if upper$ (left$ (a$, 15) ) = "CONTENT-LENGTH:" then l= val (mid$ (a$, 16) )
wend

if debug then print #2, string$(70, "-")

if debug then if l <> 0 then print #2, "LENGTH: "; l

if isChunked then if debug then print #2, "CHUNKED" : print #2
if isChunked then goto readchunked

if l <> 0 then goto readfixed

rem Read the content

while not eof (1)
	line input #1, a$
	print a$
wend
goto readend

label readfixed

gosub readblock

goto readend

label readchunked

line input #1, a$
if len (a$) > 0 then if right$ (a$, 1) = chr$ (13) then a$ = left$ (a$, len (a$) - 1)
l= val ("&" + a$)

if debug then print #2, "<--"; a$; "--> CHUNK of "; l; " bytes"

if l = 0 then goto readend

gosub readblock

REM Pass the cr/lf.
line input #1, a$

goto readchunked

label readend

close 1

end

label readblock

repeat
	in$= input$ (l, #1)
	if debug then print #2: print #2, "Block of "; len (in$); " bytes read."
	print in$;
	l= l - len (in$)
until l = 0

return

label no_host

print #2, "ERROR: Host no disponible"
end

rem End of htget.bas
