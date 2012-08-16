bin_PROGRAMS = ebview

AM_CPPFLAGS= @EBCONF_PTHREAD_CPPFLAGS@ @EBCONF_EBINCS@ \
             @EBCONF_ZLIBINCS@ @EBCONF_INTLINCS@ \
             @DISABLE_DEPRECATED@
AM_CFLAGS = @PANGOX_CFLAGS@ @GTK_CFLAGS@ @EBCONF_PTHREAD_CFLAGS@ @CYGWIN_CFLAGS@
AM_CXXFLAGS = @PANGOX_CFLAGS@ @GTK_CFLAGS@ @EBCONF_PTHREAD_CFLAGS@
ebview_LDADD = @PANGOX_LIBS@ @GTK_LIBS@ \
               @EBCONF_EBLIBS@ @EBCONF_ZLIBLIBS@ @EBCONF_INTLLIBS@ \
               @CYGWIN_LDADD@
ebview_LDFLAGS = @EBCONF_PTHREAD_LDFLAGS@ @CYGWIN_LDFLAGS@

ebview_SOURCES = \
	bmh.c \
	bmh.h \
	cellrendererebook.c \
	cellrendererebook.h \
	defs.h \
	dialog.c \
	dialog.h \
	dictbar.c \
	dictbar.h \
	dirtree.c \
	dirtree.h \
	dump.c \
	dump.h \
	eb.c \
	eb.h \
	ebview.c \
	ebview.h \
	external.c \
	external.h \
	filter.c \
	filter.h \
	global.h \
	grep.c \
	grep.h \
	headword.c \
	headword.h \
	history.c \
	history.h \
	hook.c \
	hook.h \
	intl.h \
	jcode.c \
	jcode.h \
	link.c \
	link.h \
	log.c \
	log.h \
	mainmenu.c \
	mainmenu.h \
	mainwindow.c \
	mainwindow.h \
	menu.c \
	menu.h \
	misc.c \
	misc.h \
	multi.c \
	multi.h \
	pixmap.c \
	pixmap.h \
	popup.c \
	popup.h \
	pref_color.c \
	pref_color.h \
	pref_dictgroup.c \
	pref_dictgroup.h \
	pref_dirgroup.c \
	pref_dirgroup.h \
	preference.c \
	preference.h \
	pref_external.c \
	pref_external.h \
	pref_font.c \
	pref_font.h \
	pref_grep.c \
	pref_grep.h \
	pref_gui.c \
	pref_gui.h \
	pref_io.c \
	pref_io.h \
	pref_search.c \
	pref_search.h \
	pref_selection.c \
	pref_selection.h \
	pref_shortcut.c \
	pref_shortcut.h \
	pref_stemming.c \
	pref_stemming.h \
	pref_weblist.c \
	pref_weblist.h \
	reg.c \
	reg.h \
	render.c \
	render.h \
	selection.c \
	selection.h \
	shortcut.c \
	shortcutfunc.c \
	shortcutfunc.h \
	shortcut.h \
	splash.c \
	splash.h \
	statusbar.c \
	statusbar.h \
	textview.c \
	textview.h \
	thread_search.c \
	thread_search.h \
	websearch.c \
	websearch.h \
	xml.c \
	xml.h \
	xmlinternal.c \
	xmlinternal.h

ebview.res:	ebview.rc
	windres -i $< -O coff -o $@
