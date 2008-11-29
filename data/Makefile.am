data = about.jp about.en endinglist.xml endinglist-ja.xml searchengines.xml shortcut.xml filter.xml

all: $(data)

check: all

install:
	if test -r $(MKINSTALLDIRS); then \
		$(MKINSTALLDIRS) $(pkgdatadir); \
	else \
		$(top_srcdir)/mkinstalldirs $(pkgdatadir); \
	fi; \
	data="$(data)"; \
	for file in $$data; do \
		$(INSTALL_DATA) $(srcdir)/$$file $(pkgdatadir)/$$file; \
	done;

# Define this as empty until I found a useful application.
installcheck:

uninstall:
	data="$(data)"; \
	for file in $$data; do \
	  rm -f $(pkgdatadir)/$$file; \
	done

mostlyclean:
	rm -f *.a *.o *.lo core core.* *~

clean: mostlyclean

distclean: clean
	rm -f Makefile about.jp about.en

