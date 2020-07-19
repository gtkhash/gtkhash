peonyextensionsdir = $(datadir)/peony/extensions
peonyextensions_DATA = libgtkhash-properties-peony.peony-extension

EXTRA_DIST = $(peonyextensions_DATA).in

CLEANFILES = $(peonyextensions_DATA)

%.peony-extension: %.peony-extension.in
	$(AM_V_GEN)$(MSGFMT) --desktop --keyword Name --keyword Description --template $< -d $(top_srcdir)/po -o $@
