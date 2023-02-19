10 print "Probando"
20 if osfamily$ = "windows" then co$= "find" else co$= "grep"
30 popen co$ + " ""PRINT"" > t.txt" for output as 1
40 list #1
50 close 1
60 print "Fin."
70 end
