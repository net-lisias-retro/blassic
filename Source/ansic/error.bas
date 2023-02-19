' error.bas
' Check the error control.

ON ERROR GOTO div_zero

a= 10: b= 0: PRINT a / b
a%= 10: b%= 0: PRINT a% / b%
GOTO check_syntax

LABEL div_zero
IF ERR = 7 THEN PRINT "Catched!": b= 2: b%= 2: RESUME

LABEL unexpected
PRINT "Error "; ERR; " en linea "; ERL
EXIT 1

LABEL check_syntax
ON ERROR GOTO syntax_error

INPUT A$%
PRINT A #
OPEN "t.txt" FOR INPUT 1
Hello, how are you?

GOTO check_mismatch

LABEL syntax_error
IF ERR = 1 THEN PRINT "Catched!" : RESUME NEXT

GOTO unexpected

LABEL check_mismatch
ON ERROR GOTO mismatch_error

PRINT 1 - "Hola"

END

LABEL mismatch_error
IF ERR = 2 THEN PRINT "Catched!" : RESUME NEXT

GOTO unexpected

END ' Program
