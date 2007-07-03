AUTHOR	= Roberto Alsina
TEMPLATE	= lib
DEFINES	= __CD__
EMAIL	= ralsina@kde.org
INCLUDEPATH	= ../include /usr/include/freetype2
CONFIG	+= lib qt
DESTDIR	= ../../lib/

SOURCES	= cd0emf.c \
	cd0prn.c \
	cd0wmf.c \
	cd.c \
	cdcgm.c \
	cddgn.c \
	cddxf.c \
	cdfontex.c \
	cdirgb.c \
	cdmf.c \
	cdps.c \
	cdqt.cpp \
	cgm.c \
	bparse.c \
	circle.c \
	ellipse.c \
	intcgm1.c \
	intcgm2.c \
	intcgm4.c \
	intcgm6.c \
	list.c \
	rgb2map.c \
	sim.c \
	simclip.c \
	sism.c \
	tparse.c \
	truetype.c \
	vectortext.c \
	wd.c \
	wdhdcpy.c \
	cdqtdbuffer.cpp \
	cdqtimg.cpp

HEADERS	= bparse.h \
	cdfontex.h \
	cdmfpriv.h \
	cdqt.h \
	cgm.h \
	circle.h \
	ellipse.h \
	intcgm2.h \
	intcgm4.h \
	intcgm6.h \
	intcgm.h \
	list.h \
	sim.h \
	sism.h \
	tparse.h \
	truetype.h \
	types.h

