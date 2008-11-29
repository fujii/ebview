bin_PROGRAMS = ebview

AM_CPPFLAGS= @EBCONF_PTHREAD_CPPFLAGS@ @EBCONF_EBINCS@ \
             @EBCONF_ZLIBINCS@ @EBCONF_INTLINCS@
AM_CFLAGS = @GTK_CFLAGS@ @EBCONF_PTHREAD_CFLAGS@  @CYGWIN_CFLAGS@ -Wall
AM_CXXFLAGS = @GTK_CFLAGS@ @EBCONF_PTHREAD_CFLAGS@
ebview_LDADD = @GTK_LIBS@ @THREAD_LIBS@ @CYGWIN_CFLAGS@ \
               @EBCONF_EBLIBS@ @EBCONF_ZLIBLIBS@ @EBCONF_INTLLIBS@ @RES_FILE@ @EXTRA_LIBS@
ebview_LDFLAGS = @EBCONF_PTHREAD_LDFLAGS@

ebview_SOURCES = \
	bmh.c \
	cellrendererebook.c \
	dialog.c \
	dictbar.c \
	dirtree.c \
	dump.c \
	eb.c \
	ebview.c \
	external.c \
	filter.c \
	grep.c \
	headword.c \
	history.c \
	hook.c \
	jcode.c \
	link.c \
	log.c \
	mainmenu.c \
	mainwindow.c \
	menu.c \
	misc.c \
	multi.c \
	pixmap.c \
	popup.c \
	preference.c \
	pref_color.c \
	pref_dictgroup.c \
	pref_dirgroup.c \
	pref_external.c \
	pref_font.c \
	pref_grep.c \
	pref_gui.c \
	pref_io.c \
	pref_search.c \
	pref_selection.c \
	pref_shortcut.c \
	pref_stemming.c \
	pref_weblist.c \
	reg.c \
	render.c \
	selection.c \
	shortcut.c \
	shortcutfunc.c \
	splash.c \
	statusbar.c \
	textview.c \
	thread_search.c \
	websearch.c \
	xml.c \
	xmlinternal.c

ebview.res:	ebview.rc
	windres -i $< -O coff -o $@
