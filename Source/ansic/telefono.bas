    100 REM                   Guía telefónica
    110 REM                 A. Domingo A. 1984
    120 REM Publicada en El Ordenador Personal Nº 42, Noviembre del 85
    130 REM Retoques Agosto 2001 Julián Albo
    280 REM **********************************************************
    290 REM
    300 REM color 7, 0, 1
    310 CLS
    320 REM
    330 REM color 4, 0, 1
    340 LOCATE  3, 32: PRINT "Guía telefónica-1984"
    350 LOCATE  5,  5: PRINT "1.-": LOCATE  5, 45: PRINT "2.-"
    360 LOCATE  7,  5: PRINT "3.-": LOCATE  7, 45: PRINT "4.-"
    370 LOCATE  9,  5: PRINT "5.-": LOCATE  9, 45: PRINT "6.-"
    380 REM color 2, 0, 1
    390 LOCATE  5,  8: PRINT "Instrucciones generales"
    400 LOCATE  5, 48: PRINT "Introducción de datos"
    410 LOCATE  7,  8: PRINT "Extracción de datos (nombre)"
    420 LOCATE  7, 48: PRINT "Extracción de datos (teléfono)"
    430 LOCATE  9,  8: PRINT "Visualización de datos (total)"
    440 LOCATE  9, 48: PRINT "Interrupción del programa"
    450 REM color 4, 0, 1
    460 LOCATE 12, 30: PRINT "Introduzca su opción (1 a 6)"
    470 REM color 2, 0, 1
    480 a$= INKEY$: IF a$ = "" THEN GOTO 480
    485 REM INPUT ""; a$ : a$= LEFT$ (a$, 1): IF a$ = "" THEN GOTO 460
    490 IF a$ < "1" OR a$ > "6" THEN GOTO 300
    500 REM IF a$ = "6" THEN color 7, 0, 1: CLS : END
    501 IF a$ = "6" THEN CLS: END
    510 IF a$ = "1" THEN GOTO 570
    520 IF a$ = "2" THEN GOTO 770
    530 IF a$ = "3" THEN GOTO 950
    540 IF a$ = "4" THEN GOTO 1250
    550 IF a$ = "5" THEN GOTO 1570
    560 REM
    570 REM Instrucciones generales
    580 REM
    590 REM color 4, 0, 1: CLS
    591 CLS
    600 LOCATE 3, 25: PRINT "Instrucciones generales"
    610 REM color 2, 0, 1
    620 PRINT
    630 PRINT "Ya las escribiré otro día"
    720 LOCATE 15, 30: PRINT "Pulse cualquier tecla para continuar"
    740 a$= INKEY$ : IF a$ = "" THEN 740
    741 REM INPUT ""; a$
    750 GOTO 300
    760 REM
    770 REM
    780 REM **Introducción de datos...(P-1)
    790 REM
    800 REM CLS: color 4, 0, 1
    801 CLS
    810 LOCATE 3, 30: PRINT "Introducción de datos"
    820 REM color 2, 0, 1
    830 PRINT
    840 OPEN "TELEF" FOR APPEND AS #1
    850 REM INPUT "Nombre y apellidos (FIN=acabar): "; n$
    851 PRINT "Nombre y apellidos (FIN=acabar): "; : LINE INPUT n$
    860 REM IF n$ = "FIN" OR n$ = "fin" THEN CLOSE : GOTO 300
    861 IF UPPER$ (n$) = "FIN" THEN CLOSE : GOTO 300
    880 REM INPUT "Dirección:                       "; d$
    881 PRINT "Dirección:                       "; : LINE INPUT d$
    890 REM INPUT "Provincia, prefijo y C.P.:       "; p$
    891 PRINT "Provincia, prefijo y C.P.:       "; : LINE INPUT p$
    900 REM INPUT "Teléfono:                        "; t$
    901 PRINT "Teléfono:                        "; : LINE INPUT t$
    910 REM INPUT "Comentarios:                     "; c$
    911 PRINT "Comentarios:                     "; : LINE INPUT c$
    920 WRITE #1, n$, d$, p$, t$, c$
    921 rem PRINT #1, """"; n$; ""","""; d$; ""","""; p$; ""","""; t$; ""","""; c$; """"
    930 PRINT
    940 GOTO 850
    950 REM
    960 REM ** Extracción de datos (nombre)...(P-2)
    970 REM
    980 REM CLS: color 4, 0, 1
    981 CLS
    990 LOCATE 3, 30: PRINT "Extracción de datos (1)"
   1000 REM color 2, 0, 1
   1010 PRINT
   1020 PRINT "    Opción 1: Extracción de datos por medio del nombre."
   1030 PRINT
   1040 REM color 4, 0, 1
   1050 PRINT "Nombre.-"
   1060 REM color 2, 0, 1
   1070 LOCATE 7, 10: INPUT nom$
   1080 LET l= LEN (nom$)
   1090 OPEN "TELEF" FOR INPUT AS #1
   1100 IF EOF (1) THEN CLOSE ELSE GOTO 1160
   1110 REM PRINT: color 4, 0, 1
   1111 PRINT
   1120 PRINT "Esto es todo, pulse cualquier tecla para continuar."
   1130 REM color 2, 0, 1
   1140 a$= INKEY$: IF a$= "" THEN GOTO 1140
   1141 REM INPUT ""; a$
   1150 GOTO 300
   1160 INPUT #1, n$, d$, p$, t$, c$
   1170 IF LEFT$ (n$, l) <> nom$ THEN GOTO 1240
   1180 GOSUB 2000
   1240 GOTO 1100
   1250 REM
   1260 REM ** Extracción de datos (teléfono)...(P-3)
   1270 REM
   1280 CLS
   1290 REM color 4, 0, 1
   1300 LOCATE 3, 30: PRINT "Extracción de datos (2)"
   1310 REM color 2, 0, 1
   1320 PRINT
   1330 PRINT "   Opción 2: Extracción de datos por medio del teléfono."
   1340 PRINT
   1350 REM color 4, 0, 1
   1360 PRINT "Teléfono.-"
   1370 REM color 2, 0, 1
   1380 LOCATE 7, 12: INPUT tel$
   1390 LET g= LEN (tel$)
   1400 OPEN "TELEF" FOR INPUT AS #1
   1410 IF EOF (1) THEN CLOSE ELSE GOTO 1470
   1420 REM PRINT: color 4, 0, 1
   1421 PRINT
   1430 PRINT "Esto es todo. Pulse cualquier tecla para continuar."
   1440 REM color 2, 0, 1
   1450 a$= INKEY$: IF a$ = "" THEN GOTO 1450
   1451 REM INPUT ""; a$
   1460 GOTO 300
   1470 INPUT #1, n$, d$, p$, t$, c$
   1480 IF LEFT$ (t$, g) <> tel$ THEN GOTO 1560
   1490 GOSUB 2000
   1560 GOTO 1410
   1570 REM
   1580 REM ** Extracción de datos (total)...(P-4)
   1590 REM
   1600 REM CLS: color 4, 0, 1
   1601 CLS
   1610 LOCATE 3, 30: PRINT "Extracción de datos (3)."
   1620 REM color 2, 0, 1
   1630 PRINT
   1640 PRINT "    Extracción secuencial de datos (por orden de introducción)"
   1650 PRINT
   1660 PRINT " 1.-Pulse INTRO para comenzar la extracción."
   1690 PRINT " -------------------------------------------"
   1700 PRINT " -------------------------------------------"
   1710 PRINT
   1720 REM a$= INKEY$ : IF a$ = "" THEN 1720
   1721 INPUT ""; a$
   1730 OPEN "TELEF" FOR INPUT AS #1
   1740 IF EOF (1) THEN CLOSE ELSE GOTO 1800
   1750 REM PRINT: color 4, 0, 1
   1751 PRINT
   1760 PRINT "Esto es todo, pulse cualqier tecla para continuar."
   1770 REM color 2, 0, 1
   1780 a$= INKEY$ : IF a$ = "" THEN 1780
   1781 REM INPUT ""; a$
   1790 GOTO 300
   1800 INPUT #1, n$, d$, p$, t$, c$
   1810 GOSUB 2000
   1870 GOTO 1740
   1880 REM
   1890 REM
   1900 REM ** Fin del programa (original)
   1910 END
   2000 PRINT "Nombre y apellidos: "; n$
   2010 PRINT "Dirección:          "; d$
   2020 PRINT "Provincia:          "; p$
   2030 PRINT "Teléfono:           "; t$
   2040 PRINT "Comentarios:        "; c$
   2050 PRINT "--------------------"
   2060 RETURN
   2070 REM Fin del programa, de verdad.
