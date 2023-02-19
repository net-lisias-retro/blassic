10 rem ************************
20 rem *    El asno rojo      *
30 rem *  Autor Jerome Krust  *
40 rem *  Ordenador Personal  *
50 rem *    y el autor        *
60 rem ************************
62 rem Original para Apple II publicado en El Ordenador Personal num. 51, Agosto-septiembre 1986
63 rem Adaptacion a Blassic por Julian Albo.
64 rem -------------------------------------------------------------------
65 rem Notas sobre la adaptacion.
66 rem Los efectos sonoros se han suprimido.
67 rem Las sentencias ON se han adaptado al ser en el Basic del Apple el
69 rem valor logico verdadero igual a 1 y en Blassic -1.
70 rem HPLOT se ha sustituido por PLOT.
71 rem El trazado de la cruz mediante XDRAW se ha sustituido por dos rayas
72 rem cruzadas en modo xor.
73 rem Se ha añadido el control de ctrl-c cuando no esta ennganchado, para
74 rem comodidad del jugador.
75 rem El control de las teclas mediante el codigo del caracter se ha
76 rem cambiado para permitir usar las teclas multicaracter de Blassic.
77 rem Por lo demas se ha procurado respetar el original al maximo.
78 rem -------------------------------------------------------------------
80 mode 1
90 pau= 50
110 rem
160 rem Valores representativos de los bloques almacenados en el cuadro de variables p (px, py)
170 data 8, 10, 10, 9, 8, 10, 10, 9, 5, 7, 7, 6, 5, 3, 4, 6, 0, 1, 2, 0
190 rem Almacenamiento de los datos
200 for i= 1 to 5: for j= 1 to 4: read p (i, j): next j: next i
210 'goto 270 ' Durante la depuracion
220 rem Presentacion del juego
230 a1$= "El Ordenador Personal y Jerome Krust": a2$= " presenta ...": a3$= "El asno rojo !!!"
240 cls: for i= 1 to 40: print mid$ (a1$, i, 1);: pause pau: next: print: for i= 1 to 40: print "-";: pause pau: next
260 rem Captura de las teclas de desplazamiento
270 print: print: print "Que tecla para la izquierda ?";: get g$: print: print "Que tecla para la derecha ?";: get d$
280 print: print "Que tecla para abajo ?";: get b$: print: print "Que tecla para arriba ?";: get h$
290 print: print "Que tecla para el enganche ?";: get v$
300 'goto 400 ' Durante la depuracion
330 print: print: print: for i= 1 to 16: print mid$ (a2$, i, 1);: pause pau: next: print: print
340 print: print tab (13);: for i= 1 to 15: print mid$ (a3$, i, 1);: pause pau:next
350 print: print: print "Ayuda al asno rojo a ganar la salida de su encierro, pero ten paciencia porque la anchura del camino no le simplificara la tarea ..."
360 pause 5000
380 rem Dibujo en hgr del juego
390 rem Trazado del recuadro
400 cls: plot 75, 0: for i= 77 to 203 step 2: plot to i, 0: next
410 for i= 0 to 158 step 2: plot to 203, i: next: for i= 202 to 164 step -2: plot to i, 158: next: plot 114, 158
420 for i= 115 to 75 step -2: plot to i, 158: next
430 for i= 157 to 1 step -2: plot to 75, i: next
440 plot to 202, 1 to 202, 157 to 164, 157: plot 115, 157 to 76, 157 to 76, 1
460 rem Trazado de los bloques
470 for i= 9 to 59: plot 84, i to 104, i: plot 114, i to 164, i: plot 174, i to 194, i: next
480 for i= 69 to 89: plot 84, i to 104, i: plot 114, i to 164, i: plot 174, i to 194, i: next
490 for i= 90 to 98: plot 84, i to 104, i: plot 174, i to 194, i: next
500 for i= 99 to 119: plot 84, i to 104, i: plot 114, i to 134, i: plot 144, i to 164, i: plot 174, i to 194, i: next
510 for i= 129 to 149: plot 114, i to 134, i: plot 144, i to 164, i: next: locate 21, 16: print "Salida": x= 122: y= 137: gosub 590
520 rem No borrar esta linea, es el destino de un salto.
540 rem *******Desplazamientos********
570 locate 23, 1: print "Numero de jugadas "; c; " enganchar": locate 20, 1: get a$: on -(a$ = g$) goto 610: on -(a$ = d$) goto 630: on -(a$ = b$) goto 650: on -(a$ = h$) goto 670: on -(a$ = chr$ (3) ) goto 710: on -(a$ <> v$) goto 520: goto 690
579 get a$: gosub 590: get a$: end
580 rem Presentacion y borrado de la cruz
590 pen 15, , 1: color 15: plot x - 2, y to x + 2, y: plot x, y - 2 to x, y + 2: pen 0, , 0: color 0: return
600 rem Desplazamiento de la cruz hacia la izquierda
610 gosub 590: x= 30 * (x > 92) + x: gosub 590: goto 570
620 rem Desplazamiento de la cruz hacia la derecha
630 gosub 590: x= -30 * (x < 181) + x: gosub 590: goto 570
640 rem Desplazamiento de la cruz hacia abajo
650 gosub 590: y= -30 * (y < 136) + y: gosub 590: goto 570
660 rem Desplazamiento de la cruz hacia arriba
670 gosub 590: y= 30 * (y > 18) + y: gosub 590: goto 570
680 rem Desplazamiento de los bloques
690 on (p (5, 2) = 10) * (p (5,3) = 10) goto 950: locate 23, 1: print "Numero de jugadas "; c; " enganchar": locate 20, 1: px= (x - 62) / 30: py= (y + 13) / 30: get a$: on -(a$ = g$) goto 750: on -(a$ = d$) goto 800: on -(a$ = b$) goto 850: on -(a$ = h$) goto 900
710 rem Si estas desanimado ctrl-c para salir
720 on -(a$ <> chr$ (3) ) goto 570: cls: print "Prueba una vez mas si no tienes dolor de cabeza !": get a$: end
740 rem Desplazamiento hacia la izquierda
750 on -(px = 1 or p (py, px) = 0 or p (py, px -1) <> 0) goto 610: x1= x - 8: x2= (-30 * (p (py, px) = p (py, px + 1) ) ) + x + 12: c= c + 1
760 pz= (py - 1) * - (p (py - 1, px) = p (py, px) ) + (py + 1) * -(p (py + 1, px) = p (py, px) ): pz= -(py * (pz = 0) ) + pz: y1= (30 * (py > pz) ) + y - 8: y2= (-30 * (py< pz) ) + y + 12: on -(p (pz, px - 1) <> 0) goto 610
770 gosub 590: for i= x1 - 1 to x1 - 30 step -1: color 0: plot i, y1 to i, y2: color 15: plot x2, y1 to x2, y2: x2= x2 - 1: next
780 p (py, px - 1)= p (py, px): p (pz, px -1)= p (pz, px): p= -(p (py, px) = 10) - (p (py, px) = 7): p (py, px + p)= 0: p (pz, px + p)= 0: x= 30 * ( x > 92) + x: gosub 590: goto 690
790 rem Desplazamiento hacia la derecha
800 on -(px = 4 or p (py, px) = 0 or p (py, px + 1) <> 0) goto  630: c= c + 1: x1= (30 * (p (py, px) = p (py, px - 1) ) ) + x - 8: x2= x + 12
810 pz= - (py - 1) * (p (py - 1, px) = p (py, px) ) - (py + 1) * (p (py + 1, px) = p (py, px) ): pz= -(py * (pz = 0) ) + pz: on -(p (pz, px + 1) <> 0) goto 630: y1= (30 * (py > pz) ) + y - 8: y2= (-30 * (py < pz) ) + y + 12
820 gosub 590: for i= x2 + 1 to x2 + 30: color 0: plot i, y1 to i, y2: color 15: plot x1, y1 to x1, y2: x1= x1 + 1: next
830 p (py, px + 1)= p (py, px): p (pz, px + 1)= p (pz, px): p= -(p (py, px) = 10) - (p (py, px) = 7): p (py, px - p)= 0: p (pz, px -  p)= 0: x= -30 * (x < 181) + x: gosub 590: goto 690
840 rem Desplazamiento hacia abajo
850 on -(py = 5 or p (py, px) = 0 or p (py + 1, px) <> 0) goto 650: c= c + 1: pw= -(px - 1) * (p (py, px - 1) = p (py, px) ) - (px + 1) * (p (py, px + 1) = p (py, px) ): pw= -(px * (pw = 0) ) + pw
860 on -(p (py + 1, pw) <> 0) goto 650: x1= (30 * (px > pw) ) + x - 8: x2= (-30 * (px < pw) ) + x + 12: y1= (30 * (p (py, px) = p (py - 1, px) ) ) + y - 8: y2= y + 12
870 gosub 590: for i= y2 + 1 to y2 + 30: color 0: plot x1, i to x2, i: color 15: plot x1, y1 to x2, y1: y1= y1 + 1: next
880 p (py + 1, px)= p (py, px): p (py + 1, pw)= p (py, pw): p= p (py, px): p= -(p = 10) - (p = 9) - (p = 8) - (p = 6) - (p = 5): p (py - p, px)= 0: p (py - p, pw)= 0: y= -30 * (y < 136) + y: gosub 590: goto 690
890 rem Desplazamiento hacia arriba
900 on -(py = 1 or p (py, px) = 0 or p (py - 1, px) <> 0) goto 670: c= c + 1: pw= -(px - 1) * (p (py, px - 1) = p (py, px) ) - (px + 1) * (p (py, px + 1) = p (py, px) ): pw= -(px * (pw = 0) ) + pw
910 on -(p (py - 1, pw) <> 0) goto 670: x1= (30 * (px > pw) ) + x - 8: x2= (-30 * (px < pw) ) + x + 12: y1= y - 8: y2= (-30 * (p (py, px) = p (py + 1, px) ) ) + y + 12
920 gosub 590: for i= y1 - 1 to y1 - 30 step -1: color 0: plot x1, i to x2, i: color 15: plot x1, y2 to x2, y2: y2= y2 - 1: next
930 p (py - 1, px)= p (py, px): p (py - 1, pw)= p (py, pw): p= p (py, px): p= - (p = 10) - (p = 9) - (p = 8) - (p = 6) - (p = 5): p (py + p, px)= 0: p (py + p, pw)= 0: y= 30 * (y > 18) + y: gosub 590: goto 690
940 rem Finalmente lo ha logrado
950 cls: print "BRAVO !!!": print: print "Quieres probar de nuevo ?": get a$: on -(a$ = "S") goto 110: print "Adios": end
