rem ascii.bas

grmode= 0

if programarg$ (1) = "g" then grmode= 1

if grmode then mode 10

PRINT
FOR i= 32 TO 255
	if i mod 8 = 0 then print
	PRINT "  "; hex$(i); "= "; CHR$ (i); "  ";
NEXT
PRINT: PRINT

for i= 32 to 255
	if i mod 32 = 0 then print
	print chr$ (i);
next
print: print

if grmode then get a$

END
