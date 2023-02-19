# Blassic :: Change Log

* 2003-0114: 0.5.0 (Julián Albo)
	+ Speed improvement.
	+ "PRINT , , 1" now is valid syntax. Same in WRITE.
	+ Better implementation of FILES.
	+ XPOS, YPOS, PEEK16 and PEEK32 functions.
	+ DRAWR, PLOTR, MOVER, POKE16, POKE32, RENUM, CIRCLE, MASK, WINDOW and GRAPHICS instructions.
	+ Completed the syntax of PLOT, PLOTR, DRAW, DRAWR, MOVE and MOVER with ink and ink mode parameters.
	+ Adapated some instructions to work with text windows.
	+ MODE string, when string can be "spectrum" or "cpc2"
	+ Bug PRINT AT invert arguments, corrected.
	+ Bug PRINT AT does not always work, corrected.
	+ Added variant: VAL evaluates expressions (can run more Sinclair ZX family programs). Activable with "POKE SYSVARPTR + 25, 1".
	+ New command line options -m mode and -d.
	+ Added variant "Next check relaxed", needed for many ZX-81 and Spectrum programas. Activable with "POKE SYSVARPTR + 26, 1".
	+ Added variant can DIM an array already dimensioned, needed for ZX programs. Activable with "POKE SYVARPTR + 27, 1".
	+ New sample programs.
	+ Memory leak in ERASE corrected.
	+ Bug on negative integer DATA corrected.
* 2002-1215: 0.4.5 (Julián Albo)
	+ EDIT command. Line editing in AUTO and in prompt.
	+ Prompt "Ok" instead of "]" (sorry, Apple ]['s nostalgics).
* 0000-0000: 0.4.4 (Julián Albo) - Release date is unknown.
	+ Bug DATA terminated with comma, corrected.
	+ Mouse secondary button and button release.
	+ Special keys support improved.
* 0000-0000: 0.4.3 (Julián Albo) - Release date is unknown.
	+ Mouse support in graphics mode (keywords XMOUSE and YMOUSE).
	+ Special keys are recongnized in linux in text mode.
	+ The return value of the SHELL instruction is saved in a system var.
* 0000-0000: 0.4.2 (Julián Albo) - Release date is unknown.
	+ Now compiles with Kylix.
	+ Workarounds for bugs that cause crash on some platforms when a syntax error is generated.
	+ Bug ON BREAK CONT causes internal error on INPUT.
	+ Bug AUTO n causes syntax error.
* 0000-0000: 0.4.1 (Julián Albo) - Release date is unknown.
	+ TRON LINE.
	+ Channel specification in TRON.
	+ Doesn't use hash_map in gcc 3.
	+ Corrected problem in gcc 2.95.2
	+ Big numbers whitout decimal nor exponent are now handled correctly on all platforms.
* 0000-0000: 0.4.0 (Julián Albo) - Release date is unknown.
	+ Integer type variables. Integer and real suffixes.
	+ DEF STR, DEF INT, DEF REAL
	+ Comments with '
	+ No keywords in comments.
	+ MERGE and CHAIN MERGE now try .blc and .bas extensions and work with binary files.
	+ OSNAME$
	+ Tabs in graphics mode.
* 0000-0000: 0.3.5 (Julián Albo) - Release date is unknown.
	+ CHAIN, CHAIN MERGE
	+ Bug some instructions don't continue processing current line.
* 0000-0000: 0.3.4 (Julián Albo) - Release date is unknown.
	+ ON BREAK STOP / CONT / GOSUB
	+ SYNCHRONIZE, PAUSE.
	+ Bug LOCATE in graphics mode started in 0 instead of 1, corrected.
	+ Bug PRINT AT doesn't work in graphics mode, corrected.
* 0000-0000: 0.3.3 (Julián Albo) - Release date is unknown.
	+ Bug scroll & cls in invert mode corrected.
	+ Bug scroll fills white instead of current paper corrected.
	+ Bug blank or empty USING corrected.
	+ Bug INPUT only positive integer coorrected.
	+ FIX and CINT functions.
	+ Preliminary support for colors on text mode.
* 0000-0000: 0.3.2 (Julián Albo) - Release date is unknown.
	+ PAPER, PEN, SHELL, CHDIR, MKDIR and RMDIR instructions.
	+ Syntax of LINE has changed, now is like Gwbasic.
	+ Simple implementation of FILES.
	+ Text output in graphics mode now in colors and with transparent option.
	+ Error corrections to compile on C++ Builder 6.
	+ Graphics modes now works on Win 2000 and XP.
* 0000-0000: 0.3.1 (Julián Albo) - Release date is unknown.
	+ POP, NAME, KILL and FILES instructions (FILES not implemented).
	+ MIN and MAX functions.
	+ Corrected bug in Makefile.
	+ Some changes to avoid errors and warnings on some versions of gcc and C++ Builder.
	+ Some code cleanup.
	+ File not found on OPEN for input.

Previous releases are M.I.A.
