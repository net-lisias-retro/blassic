# $Id: Makefile.am,v 1.2 2002/05/24 23:07:13 sarrazip Exp $

noinst_PROGRAMS = gencharset
bin_PROGRAMS = blassic


# This tells Automake that charset.cpp must be generated before
# anything else is compiled.
BUILT_SOURCES = charset.cpp


# Needed to generate charset.cpp:
gencharset_SOURCES = gencharset.cpp

gencharset_CXXFLAGS = \
	@SVGALIB_CFLAGS@ \
	-Wall -pedantic -Wwrite-strings -Wstrict-prototypes

gencharset_LDADD =

blassic_SOURCES = \
	charset.cpp \
	charset.h \
	blassic.cpp \
	blassic.h \
	codeline.cpp \
	codeline.h \
	cursor.cpp \
	cursor.h \
	dim.cpp \
	dim.h \
	dynamic.h \
	edit.cpp \
	edit.h \
	error.cpp \
	error.h \
	file.cpp \
	file.h \
	function.cpp \
	function.h \
	graphics.cpp \
	graphics.h \
	key.cpp \
	key.h \
	keyword.cpp \
	keyword.h \
	program.cpp \
	program.h \
	result.h \
	runner.cpp \
	runner.h \
	runnerline.cpp \
	runnerline.h \
	socket.cpp \
	socket.h \
	sysvar.cpp \
	sysvar.h \
	token.cpp \
	token.h \
	trace.cpp \
	trace.h \
	util.h \
	var.cpp \
	var.h \
	version.cpp

blassic_CXXFLAGS = \
	@SVGALIB_CFLAGS@ \
	-Wall -pedantic -Wwrite-strings -Wstrict-prototypes

# Julian:
#blassic_LDADD = -ldl -lncurses @SVGALIB_LIBS@ -L/usr/X11R6/lib -lX11
# Let's configure check if libdl exist (hp-ux does noy use it, for example).
blassic_LDADD = -lncurses @SVGALIB_LIBS@ -L/usr/X11R6/lib -lX11


charset.cpp: $(srcdir)/charset.def gencharset
	./gencharset $(srcdir)/charset.def charset.cpp
	test -f charset.cpp


testdl.so: testdl.o
	gcc -shared -Wl,-soname,testdl.so -o testdl.so testdl.o
testdl.o: testdl.cpp
	gcc -Wall -fPIC -c testdl.cpp

EXAMPLE_PROGS = \
	4raya.bas anime.bas anime2.bas args.bas ascii.bas asnorojo.bas \
	automod.bas autoview.bas batnum.bas bench.bas bingrep.bas \
	biorritmo.bas bubble.bas chart.bas circle.bas colores.bas \
	deepsea.bas defchar.bas deriv.bas drawtest.bas eliza.bas \
	error.bas estrella.bas euler.bas europa.bas fecha.bas flip.bas \
	fnfact.bas fnfactrec.bas fnrecur.bas fractale.bas genmucha.bas \
	graficos3d.bas graph.bas hamurs.bas hockey.bas htget.bas \
	interpolacion.bas invert.bas label.bas labeldata.bas lineas.bas \
	liscua.bas list.bas machinecode.bas maildrop.bas mastermind.bas \
	memerror.bas mkcv.bas mouse.bas nonumber.bas nttp.bas \
	numbers.bas path.bas pid.bas points.bas points2.bas popenin.bas \
	popenout.bas popenout2.bas presskey.bas prueba.bas randomr.bas \
	randomw.bas raqueta.bas repeat.bas repg.bas salvo1.bas \
	shellsort.bas slowprint.bas snoopy.bas telefono.bas testdl.bas \
	testfn.bas testlocal.bas testrect.bas textcolor.bas \
	textpaper.bas trim.bas viewpass.bas while.bas window.bas

if INSTALL_EXAMPLE_PROGS
examplesdir = $(datadir)/@PACKAGE@/examples
examples_DATA = $(EXAMPLE_PROGS)
endif

rpm: dist
	rpm -ta $(distdir).tar.gz

EXTRA_DIST = \
	bootstrap autogen.sh \
	blassic.spec \
	svgalib.m4 \
	alphabet.blc automod.blc \
	blassic.bpr \
	testdl.bpr \
	random.dat \
	charset.def \
	counter.sh \
	testdl.cpp \
	$(EXAMPLE_PROGS)

CLEANFILES = testdl.so $(BUILT_SOURCES)

# When Automake needs to regenerate configure, the following options
# will be passed to aclocal, as in the bootstrap script.
ACLOCAL_AMFLAGS = -I .


# Boilerplate:

auxdir = @ac_aux_dir@
AUX_DIST = $(auxdir)/install-sh $(auxdir)/missing $(auxdir)/mkinstalldirs

MAINTAINERCLEANFILES = \
	Makefile.in \
	aclocal.m4 \
	configure \
	sic/config-h.in \
	sic/stamp-h.in \
	$(AUX_DIST) \
	depcomp \
	config.guess \
	config.log \
	config.status \
	config.sub \
	install-sh \
	missing \
	mkinstalldirs

