REM
REM List a binary blassic program.
REM

rem DEF FN peekvalue (p$, a)=asc (mid$ (p$, a, 1) ) + 256 * (asc (mid$ (p$, a + 1, 1) ) + 256 * (asc (mid$ (p$, a + 2, 1) ) + 256 * asc (mid$ (p$, a + 3, 1) ) ) )

DEF FN peekvalue (a)=asc (mid$ (prog$, a, 1) ) + 256 * (asc (mid$ (prog$, a + 1, 1) ) + 256 * (asc (mid$ (prog$, a + 2, 1) ) + 256 * asc (mid$ (prog$, a + 3, 1) ) ) )

GOTO init

LABEL peekvalue

r= ASC (MID$ (prog$, a, 1) ) + 256 * (ASC (MID$ (prog$, a + 1, 1) ) + 256 * (ASC (MID$ (prog$, a + 2, 1) ) + 256 * ASC (MID$ (prog$, a + 3, 1) ) ) )
RETURN

LABEL decodeline

FOR i= p + 8 TO p + l + 7
	c= ASC (MID$ (prog$, i, 1) )
	IF c = 1 OR c = 2 OR c = 3 OR c = 4 THEN GOTO iscode
	IF c = 5 THEN GOTO isinteger
	IF c <> ASC ("""") THEN GOTO noliteral

	li$= li$ + """" : i= i + 1
	WHILE MID$ (prog$, i, 1) <> CHR$ (0)
		IF MID$ (prog$, i, 1) <> """" THEN li$= li$ + MID$ (prog$, i, 1) ELSE li$= li$ + """"""
		i= i + 1
	WEND
	li$= li$ + """"
	GOTO nextchar

	LABEL noliteral

	li$= li$ + MID$ (prog$, i, 1)
	GOTO nextchar

	LABEL isinteger
	num%= fn peekvalue (i+1)
	li$= li$ + str$ (num%)
	i= i + 4
	GOTO nextchar

	LABEL iscode

	code= 256 * ASC (MID$ (prog$, i, 1) ) + ASC (MID$ (prog$, i + 1, 1) )
	GOSUB gettoken
	li$= li$ + token$
	i= i + 1

	LABEL nextchar
NEXT

RETURN


REM Bucle principal

LABEL init

progname$= PROGRAMARG$ (1)

IF progname$ = "-n" then progname$= PROGRAMARG$ (2) : else numbers= 1

IF progname$ = "" THEN PRINT "Falta argumento" : END

LOAD progname$, prog$

IF LEFT$ (prog$, 8) <> "Blassic" + CHR$ (0) THEN PRINT "No es un binario Blassic" : END

gosub loadtable

endprog= LEN (prog$)
p= 17 : REM Salta la identificacion y cabecera

WHILE p < endprog
	rem a= p: GOSUB peekvalue: n= r
	rem n= FN peekvalue (prog$, p)
	n= FN peekvalue (p)
	li$= ""
	if numbers then li$= RIGHT$ ("       " + str$ (n), 7) + " "
	rem a= p + 4: GOSUB peekvalue: l= r
	rem l= FN peekvalue (prog$, p + 4)
	l= FN peekvalue (p + 4)
	IF l > 0 THEN GOSUB decodeline
	PRINT li$
	p= p + 8 + l
WEND

END

REM Carga la tabla de codigos

LABEL loadtable

numcod= 180
dim name$ (numcod), cod (numcod)
for i= 1 to numcod
	read name$ (i), cod (i)
	rem print name$ (i), cod (i)
next
read check$ : if check$ <> "***" then print "ERROR EN DATAS" : end
return


REM Obtiene el token correspondiente al codigo

LABEL gettoken

res= 0
for busca= 1 to numcod
	if cod (busca) = code then res= busca : busca= numcod
next
if res = 0 then token$= "???" else token$= name$ (res)
return

REM Comandos

DATA "END", 257
DATA "LIST", 258
DATA "REM", 259
DATA "LOAD", 260
DATA "SAVE", 261
DATA "NEW", 262
DATA "EXIT", 263
DATA "RUN", 264
DATA "PRINT", 265
DATA "FOR", 266
DATA "NEXT", 267
DATA "TO", 268
DATA "STEP", 269
DATA "IF", 270
DATA "THEN", 271
DATA "ELSE",272
DATA "TRON",273
DATA "TROFF", 274
DATA "LET", 275
DATA "GOTO", 276
DATA "STOP",277
DATA "CONT", 278
DATA "CLEAR", 279
DATA "GOSUB", 280
DATA "RETURN", 281
DATA "POKE", 282
DATA "DATA",283
DATA "READ",284
DATA "RESTORE",285
DATA "INPUT", 286
DATA "LINE", 287
DATA "RANDOMIZE", 288
DATA "AUTO", 290
DATA "DIM", 291
DATA "SYSTEM", 292
DATA "ON", 293
DATA "ERROR", 294
DATA "OPEN", 295
DATA "CLOSE", 296
DATA "OUTPUT", 297
DATA "AS", 298
DATA "LOCATE", 299
DATA "CLS", 300
DATA "APPEND", 301
DATA "WRITE", 302
DATA "MODE", 303
DATA "MOVE", 304
DATA "COLOR", 305
DATA "GET", 306
DATA "LABEL", 307
DATA "DELIMITER", 308
DATA "REPEAT", 309
DATA "UNTIL", 310
DATA "WHILE", 311
DATA "WEND", 312
DATA "PLOT", 313
DATA "POPEN", 314
DATA "RESUME", 315
DATA "DELETE", 316
DATA "LOCAL", 317
DATA "RANDOM", 318
DATA "PUT", 319
DATA "FIELD", 320
DATA "LSET", 321
DATA "RSET", 322
DATA "SOCKET", 323
DATA "DRAW", 324
DATA "DEF", 325
DATA "FN", 326
DATA "ERASE", 327
DATA "SWAP", 328
DATA "SYMBOL", 329
DATA "ZONE", 330
DATA "POP", 331
DATA "NAME", 332
DATA "KILL", 333
DATA "FILES", 334
DATA "PAPER", 335
DATA "PEN", 336
DATA "SHELL", 337
DATA "MERGE", 338
DATA "CHDIR", 339
DATA "MKDIR", 340
DATA "RMDIR", 341
DATA "BREAK", 342
DATA "SYNCHRONIZE", 343
DATA "PAUSE", 344
DATA "CHAIN", 345
DATA "STR", 346
DATA "REAL", 347
DATA "ENVIRON", 348
DATA "EDIT", 349
DATA "DRAWR", 350
DATA "PLOTR", 351
DATA "MOVER", 352
DATA "POKE16", 353
DATA "POKE32", 354
DATA "RENUM", 355
DATA "CIRCLE", 356
DATA "MASK", 357
DATA "WINDOW", 358
DATA "GRAPHICS", 359

REM Funciones de cadena

DATA "MID$",513
DATA "LEFT$",514
DATA "RIGHT$",515
DATA "CHR$",516
DATA "ENVIRON$", 517
DATA "STRING$", 518
DATA "OSFAMILY$", 519
DATA "HEX$", 520
DATA "SPACE$", 521
DATA "UPPER$", 522
DATA "LOWER$", 523
DATA "STR$", 524
DATA "OCT$", 525
DATA "BIN$", 526
DATA "INKEY$", 527
DATA "PROGRAMARG$", 528
DATA "DATE$", 529
DATA "TIME$", 530
DATA "INPUT$", 531
DATA "MKI$", 532
DATA "MKS$", 533
DATA "MKD$", 534
DATA "MKL$", 535
DATA "TRIM$", 536
DATA "LTRIM$", 537
DATA "RTRIM$", 538
DATA "OSNAME$", 539
REM Funciones numericas

DATA "ASC", 769
DATA "LEN", 770
DATA "PEEK",771
DATA "PROGRAMPTR",772
DATA "RND",773
DATA "INT",774
DATA "SIN",775
DATA "COS",776
DATA "PI",777
DATA "TAN", 778
DATA "SQR", 779
DATA "ASIN", 780
DATA "ACOS", 781
DATA "INSTR", 782
DATA "ATAN", 783
DATA "ABS", 784
DATA "USR", 785
DATA "VAL", 786
DATA "EOF", 787
DATA "VARPTR", 788
DATA "SYSVARPTR", 789
DATA "SGN", 790
DATA "LOG", 791
DATA "LOG10", 792
DATA "EXP", 793
DATA "TIME", 794
DATA "ERR", 795
DATA "ERL", 796
DATA "CVI", 797
DATA "CVS", 798
DATA "CVD", 799
DATA "CVL", 800
DATA "MIN", 801
DATA "MAX", 802
DATA "CINT", 803
DATA "FIX", 804
DATA "XMOUSE", 805
DATA "YMOUSE", 806
DATA "XPOS", 807
DATA "YPOS", 808
DATA "PEEK16", 809
DATA "PEEK32", 810

REM Operadores

DATA "NOT", 1025
DATA "OR", 1026
DATA "AND", 1027
DATA "TAB", 1028
DATA "SPC", 1029
DATA "AT", 1030
DATA "XOR", 1031
DATA "MOD", 1032
DATA "USING", 1033
DATA "***"

REM Fin
