10 input "Enter your name"; a$: mode "cpc2"
20 print "You certainly get around "; a$
30 tag
40 x= len (a$) * 9: y= 50 + rnd * 300: move -x, y
50 for f= -x to 640 step rnd * 6 + 2
60 move f, y: print " "; a$;: pause 10:next
70 for b= 640 to -x step -rnd * 6 - 2
80 move b, y: print a$; " ";:pause 10: next
90 if inkey$ = "" then goto 40
