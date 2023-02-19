REM viewpass.bas
REM View /etc/passwd

IF OSFAMILY$ <> "unix" THEN PRINT "This program is only for unix" : END

userfind$= programarg$ (1)
IF userfind$ = "" THEN userfind$= ENVIRON$ ("LOGNAME")
IF userfind$ = "" THEN PRINT "User not specified": END

OPEN "/etc/passwd" FOR INPUT AS #1
DELIMITER #1, ":"
userfound= 0
WHILE NOT EOF (1) AND NOT userfound
	INPUT #1, user$, pass$, uid%, gid%, name$, home$, shell$
	IF user$ = userfind$ THEN userfound= -1
WEND
CLOSE #1
IF NOT userfound THEN PRINT "User not found": END

PRINT "User: "; user$; " UID: "; uid%
PRINT "Password: "; pass$

OPEN "/etc/group" FOR INPUT AS #1
DELIMITER #1, ":"
groupname$= ""
WHILE NOT EOF (1) AND groupname$ = ""
	INPUT #1, group$, gpass$, g%, member$
	IF g% = gid% THEN groupname$= group$
WEND
CLOSE #1

IF groupname$ <> "" THEN PRINT "Group: "; groupname$; " ";
PRINT "GID: "; gid%
PRINT "Name: "; name$
PRINT "Home: "; home$
PRINT "Shell: "; shell$

END
