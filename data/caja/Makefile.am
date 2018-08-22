cajaextensionsdir = $(datadir)/caja/extensions
cajaextensions_DATA = libgtkhash-properties-caja.caja-extension

EXTRA_DIST = $(cajaextensions_DATA).in

CLEANFILES = $(cajaextensions_DATA)

%.caja-extension: %.caja-extension.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po)
	$(INTLTOOL_V_MERGE)LC_ALL=C $(INTLTOOL_MERGE) $(INTLTOOL_V_MERGE_OPTIONS) -d -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $< $@
