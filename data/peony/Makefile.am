peonyextensionsdir = $(datadir)/peony/extensions
peonyextensions_DATA = libgtkhash-properties-peony.peony-extension

EXTRA_DIST = $(peonyextensions_DATA).in

CLEANFILES = $(peonyextensions_DATA)

%.peony-extension: %.peony-extension.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po)
	$(INTLTOOL_V_MERGE)LC_ALL=C $(INTLTOOL_MERGE) $(INTLTOOL_V_MERGE_OPTIONS) -d -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $< $@
