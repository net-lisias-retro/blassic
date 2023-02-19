10 rem popenout.bas
20 if osfamily$ = "windows" then co$= "find" else co$= "grep"
30 co$= co$+ " ""hola"" > t.txt"
40 print co$
50 popen co$ for output as #1
60 print #1, "Empezamos"
70 print #1, "hola que tal"
80 print #1, "Terminamos"
90 close #1
100 end
