REM Test restore with a label.

DATA 20

LABEL initdata

DATA 10

RESTORE initdata
READ a
IF A <> 10 THEN PRINT "Failed!" : EXIT 1
PRINT "Correct."

END
