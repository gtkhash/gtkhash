cajaextensionsdir = $(datadir)/caja/extensions
cajaextensions_DATA = libgtkhash-properties-caja.caja-extension

EXTRA_DIST = $(cajaextensions_DATA).in

CLEANFILES = $(cajaextensions_DATA)

%.caja-extension: %.caja-extension.in
	$(AM_V_GEN)$(MSGFMT) --desktop --keyword Name --keyword Description --template $< -d $(top_srcdir)/po -o $@
