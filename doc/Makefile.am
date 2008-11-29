dir = ja en
data = index.html menu.html body.html

all: 

check: all

install:
	dir="$(dir)"; \
	for lang in $$dir; do \
		if test -r $(MKINSTALLDIRS); then \
			$(MKINSTALLDIRS) $(pkgdatadir)/help/$$lang; \
		else \
			$(top_srcdir)/mkinstalldirs $(pkgdatadir)/help/$$lang; \
		fi; \
	done; \
	data="$(data)"; \
	for lang in $$dir; do \
		for file in $$data; do \
			$(INSTALL_DATA) $(srcdir)/$$lang/$$file $(pkgdatadir)/help/$$lang/$$file; \
		done; \
	done;

# Define this as empty until I found a useful application.
installcheck:

uninstall:
	rm -fr $(pkgdatadir)/help

