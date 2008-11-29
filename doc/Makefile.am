EXTRA_DIST = ja en

dir = ja en

install-data-local:
	$(MKDIR_P) "$(DESTDIR)$(pkgdatadir)/help"
	for lang in $(dir); do \
	  $(MKDIR_P) "$(DESTDIR)$(pkgdatadir)/help/$$lang"; \
	  for p in "$(srcdir)/$$lang/"*.html; do \
	    f=`echo $$p | sed -e 's|^.*/||'`; \
	    $(install_sh_DATA) "$$p" "$(DESTDIR)$(pkgdatadir)/help/$$lang/$$f"; \
	  done; \
	done;

uninstall-local:
	rm -rf "$(DESTDIR)$(pkgdatadir)/help"
