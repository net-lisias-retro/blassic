#!/usr/local/experimental/blassic3.2/blassic

REM BLASSIC NEWS READER
REM Autor original: Jose I. Ria~o (Aka JIR , Aka Tin de Leon)  2002/Ago/27

REM Hecho mas que nada para saber si era capaz de hacerlo :-) y probar
REM como funcionaba lo del socket y lo del programarg$ 

bnr_ver$="0.0.3"

if programarg$(1)="--version" then goto versionprint
if programarg$(1)="--help" then goto helpprint

goto normaluse

label versionprint :::::::::
? "BNR - Blassic News Reader - Version:";
? bnr_ver$
? "by Jose I.'Tin de Leon' Ria~o < jriachi@ribera.tel.uva.es> 2002"
? "BNR comes with ABSOLUTELY NO WARRANTY"
? "You may redistribute copies of BNR"
? "under the terms of the GNU General Public License version 2."
end

label helpprint ::::::::::::
? "uso: ";programarg$(0);" [ --help ]|[ --version ]|[<servidorNTTP> [<puerto>]]
end

label normaluse ::::::::::::::::::::

nttpserver$=programarg$(1)
nttpport$=programarg$(2)

if nttpserver$="" then nttpserver$="news4.euro.net"
if val(nttpport$)=0 then nttpport$="119"
? "BNR version ";bnr_ver$
socket nttpserver$ ,  val(nttpport$) as #1
line input #1, a$
? a$:?
if mid$(a$,1,3)="200" or mid$(a$,1,3)="201" then goto menugrupos
 ? "Advertencia: Parece que no somos bienvenidos!"
 ? "El servidor puede cerrar la conexion en cualquier momento...!!"

label menugrupos::::::::::
? "Que grupo desea consultar? ";
input group$
label relistar
? #1,"group "+group$
delimiter #1, " "
input #1,code,nart,iart,fart,gr$,se$:
if code=211 then goto cabeceras
if code=411 then ? "Error 411, No existe el grupo o como dijo el server: ";

REM otros errores de GROUP:::::::::::::
delimiter #1,chr$(13)+chr$(10): 
? #1,"group ";group$
line input #1,errors$
if code<>411 then ? "!!! Error, server dixit: ";
? errors$:goto menugrupos


label cabeceras::::::::::
? "El primer articulo en "; group$ ;" es el: "; iart ; " y el ultimo el ";fart
? "Indique el rango de articulos que desea listar..."
? "Cual es el primer articulo que he de listar [enter=";iart;"] ";
input diart
? "Cual es el ultimo articulo que he de listar [enter=";fart;"] ";
input dfart

if diart<>0 and diart>=iart and diart<=fart then iart=diart
if dfart<>0 and diart>=iart and dfart<=fart then fart=dfart

? "Subjects y autores:"
? "---------"
cart=iart-1
for i=iart to fart
? "Art ";i;":";
? #1,"head ";i
line input #1,a$
if mid$(a$,1,3)<>"221" then ? "ERROR; server dixit: ";a$:next
a$="":while a$<>"."+chr$(13)
line input #1,a$

if mid$(a$,1,7)="Subject" then subj$=mid$(a$,9,len(a$)-9)
if mid$(a$,1,4)="From" then fro$=mid$(a$,6,len(a$)-6)
wend
? subj$;" (";fro$;")"
next

LABEL menulectura :::::::::::::::::::::::::::::::::::::::::
? "Que articulo deseas leer (numero , o S=Siguiente R=relistar, G=otro grupo)";
input dart$
if upper$(dart$)="R" then goto relistar
if upper$(dart$)="G" then goto menugrupos
if upper$(dart$)="S" then cart=cart+1
if val(dart$)<>0 then cart=val(dart$)
if dart$="" then cart=cart+1
? #1,"article ";cart
line input #1,a$: if mid$(a$,1,3)<>"220" then ? "!!! Error: server dixit ";a$ :goto menulectura
a$="":isbody=0:while a$<>"."+chr$(13)
line input #1,a$
if mid$(a$,1,7)="Subject" and isbody=0 then subj$=mid$(a$,9,len(a$)-9)
if mid$(a$,1,4)="From" and isbody=0 then fro$=mid$(a$,6,len(a$)-6)
if mid$(a$,1,10)="Newsgroups" and isbody=0 then ng$=mid$(a$,12,len(a$)-12)
if isbody=1 then ? a$
if a$=chr$(13) and isbody=0 then isbody=1:pheader=1 
if pheader=1 then ?:?  "+";string$(72,"-"):? "| Articulo#:";cart; "(en ";group$;")" :? "| Asunto:";subj$:
if pheader=1 then ? "| Autor:";fro$:? "| Newsgroups: ";ng$: ? "+";string$(72,"-")
if pheader=1 then pheader=0
wend

gosub menulectura
