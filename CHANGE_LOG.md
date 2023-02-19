# Blassic :: Change Log

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
