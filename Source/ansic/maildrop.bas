1 REM    LECTOR DE BUZON E-MAIL
2 REM    ----------------------
3 REM    Permite leer el correo y borrar los mensajes no deseados
4 REM    Version 0.0.0.0.0.0.1.pre-alpha
5 print "Lector de Buzon E-mail, programado en Blassic":print
10 popserver$=programarg$(1)
20 if popserver$="" then print "Servidor pop:";:input popserver$
30 print "User pop:";:input popuser$
40 print "Password: ";
50 input poppass$
60 socket popserver$, 110 as #1
70 line input #1, entrada$
80 print entrada$
90 if mid$(entrada$,1,3)="+OK" then goto conectado
100 print "El servidor ";popserver$;" parece que no responde."
110 goto fin
197 rem
198 label conectado
199 rem
200 print #1, "USER ";popuser$
210 print "USER ";popuser$
220 resultado$=""
230 line input #1, resultado$
240 if mid$(resultado$,1,3)<>"+OK" then goto fin
250 print resultado$
260 print #1, "PASS ";poppass$
270 line input #1, resultado$
280 if mid$(resultado$,1,3)<>"+OK" then goto fin
290 gosub 1000
297 rem
298 label vermensaje
299 rem
300 print:print "Que mensajes deseas ver? [1-";totalnummsg;" - 0 fin]: ";
310 input nummsg$
320 if val(nummsg$)<0 or val(nummsg$)>totalnummsg then goto errornummensajes
330 if val(nummsg$)=0 or nummsg$="" then goto fin
340 nummsg=val(nummsg$)
350 cls
360 print #1, "TOP ";nummsg;" 0"+chr$(13)
370 rem print "TOP ";nummsg;" 0"+chr$(13)
380 line input #1, a$
390 if (mid$(a$,1,3)<>"+OK") then print "El mensaje numero ";nummsg;" no existe.":goto 300
400 a$=""
410 linea=0
420 repeat
430 line input #1, a$
440 if upper$(mid$(a$,1,5))="FROM:" then from$=a$
450 if upper$(mid$(a$,1,3))="TO:" then to$=a$
460 if upper$(mid$(a$,1,8))="SUBJECT:" then subject$= a$
470 linea=linea+1
480 if linea=24 then gosub 800
490 until a$="."+chr$(13)
500 if (from$<>"") then print from$ else print "No existe campo From: en cabecera."
510 if (to$<>"") then print to$ else print "No existe campo To: en cabecera."
520 if (subject$<>"") then print subject$ else print "No existe campo Subject: en cabecera."
530 a$=""
540 while (upper$(a$)<>"S" or upper$(a$)<>"N")
550 print "Deseas ver el mensaje [S/N] ";
560 input a$
570 if upper$(a$)="S" then goto 660
580 if upper$(a$)="N" then goto 600
590 wend
600 a$=""
610 while (upper$(a$)<>"S" or upper$(a$)<>"N")
620 print "Deseas borrar el mensaje [S/N] ";: input a$
630 if upper$(a$)="S" then gosub 2000:goto 300
640 if upper$(a$)="N" then goto 300
650 wend
657 rem
658 label recupera
659 rem
660 print #1, "RETR ";nummsg;chr$(13)
670 linea=0: a$=""
680 repeat
690 line input #1, a$
700 if a$="."+chr$(13) then print else print a$
710 REM print a$
720 linea=linea+1
730 if linea=24 then gosub 800
740 until a$="."+chr$(13)
750 print "Fin del mensaje": print "Deseas borrar este mensaje: ['S'=Si] ";:input borra$
760 if borra$="S" then gosub 2000
770 print "Pulsa una tecla para regresar"
780 tecla$=inkey$: if tecla$="" then 780
790 goto 300
797 rem
798 label tecla
799 rem
800 print "Pulsa una tecla para continuar....";
810 tecla$= INKEY$ : IF tecla$ = "" THEN 810
820 linea=0
830 print
840 cls
850 return
897 rem
898 rem
899 label errornummensajes
900 print "Error: El mensaje no existe."
910 goto 300
948 rem
949 label fin
950 print "Ncha!"
960 print #1, "quit"+chr$(13)
970 end
998 rem
999 rem
1000 delimiter #1, " "
1010 print #1, "STAT"
1020 input #1,total$, totalnummsg, totalbytes
1040 if mid$(total$,1,3)<>"+OK" then goto fin
1050 print:print "Hay un total de "; totalnummsg;" mensajes que ocupan un total de ";totalbytes;" ."
1060 return
1999 rem
2000 print #1, "DELE ";nummsg
2010 line input #1, a$
2020 if (mid$(a$,1,3)<>"+OK") then print "Error al borrar mensaje" else print "Mensaje ";nummsg;" borrado!."
2030 gosub 1000
2040 return
