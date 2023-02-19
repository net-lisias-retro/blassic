a= 10
a$="global"
GOSUB level1
PRINT a, a$
END

LABEL level1
LOCAL a, a$
a= 5678
a$= "level1"
GOSUB level2
PRINT a, a$
RETURN

LABEL level2
LOCAL a$, a
a= 1234
a$= "level2"
GOSUB level3
PRINT a, a$
RETURN

LABEL level3
LOCAL a
a=1357
LOCAL a$
a$= "level3"
PRINT a, a$
RETURN
