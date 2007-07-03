warning( "This file has been generated by Monkey Studio (2006.1.6.0) - http://sourceforge.net/projects/monkeystudio" )

AUTHOR	= Roberto Alsina
TEMPLATE	= lib
EMAIL	= ralsina@kde.org
QT	= gui core
LANGUAGE	= C++
INCLUDEPATH	= ../include ../iup ../cd/include
CONFIG	+= qt
DESTDIR	= ../lib/

SOURCES	= ./iupimglib.c \
	./tree/itdraw.c \
	./tree/itscroll.c \
	./tree/iuptree.c \
	./tree/itkey.c \
	./tree/itcallback.c \
	./tree/itfind.c \
	./tree/itgetset.c \
	./tree/itmouse.c \
	./tree/itlimits.c \
	./tree/itimage.c \
	./tree/itedit.c \
	./iupval.c \
	./iupgauge.c \
	./iupcontrols.c \
	./mask/iupmask.c \
	./mask/imask_match.c \
	./mask/imask_parse.c \
	./iupcells.c \
	./iupcbox.c \
	./color/icb_hls.c \
	./color/iupgc.c \
	./color/igc_lng.c \
	./color/iupcb.c \
	./color/icb_rgb.c \
	./cdiuputil.c \
	./iuptabs.c \
	./iupspin.c \
	./iupdial.c \
	./iupsbox.c \
	./iupgetparam.c \
	./matrix/imedit.c \
	./matrix/immark.c \
	./matrix/immouse.c \
	./matrix/imscroll.c \
	./matrix/imaux.c \
	./matrix/imdraw.c \
	./matrix/imcolres.c \
	./matrix/imkey.c \
	./matrix/imnumlc.c \
	./matrix/immem.c \
	./matrix/imgetset.c \
	./matrix/imfocus.c \
	./matrix/iupmatrix.c \
	./iupcolorbar.c

HEADERS	= ./icontrols.h \
	./tree/itcallback.h \
	./tree/treecd.h \
	./tree/itkey.h \
	./tree/itedit.h \
	./tree/itdraw.h \
	./tree/itgetset.h \
	./tree/itimage.h \
	./tree/itfind.h \
	./tree/itscroll.h \
	./tree/itmouse.h \
	./tree/itlimits.h \
	./tree/treedef.h \
	./mask/imask_match.h \
	./mask/imask.h \
	./cdiuputil.h \
	./color/icb_hls.h \
	./color/icb_rgb.h \
	./color/icb_tabs.h \
	./color/igc_lng.h \
	./matrix/matridef.h \
	./matrix/immem.h \
	./matrix/imkey.h \
	./matrix/imaux.h \
	./matrix/imcolres.h \
	./matrix/imnumlc.h \
	./matrix/imgetset.h \
	./matrix/imdraw.h \
	./matrix/imscroll.h \
	./matrix/imedit.h \
	./matrix/imfocus.h \
	./matrix/matrixcd.h \
	./matrix/immark.h \
	./matrix/immouse.h
