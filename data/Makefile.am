SUBDIRS = icons

if ENABLE_APPSTREAM
SUBDIRS += appdata
endif

if ENABLE_CAJA
SUBDIRS += caja
endif

if ENABLE_NAUTILUS_DATA
SUBDIRS += nautilus
endif

if ENABLE_PEONY
SUBDIRS += peony
endif

if ENABLE_GTKHASH

applicationsdir = $(datadir)/applications
applications_DATA = org.gtkhash.gtkhash.desktop

gsettings_SCHEMAS = org.gtkhash.gschema.xml

EXTRA_DIST = \
	$(applications_DATA).in \
	$(gsettings_SCHEMAS) \
	gtkhash-gtk3.ui gtkhash-gtk3.gresource.xml

CLEANFILES = $(applications_DATA)

endif

%.desktop: %.desktop.in
	$(AM_V_GEN)$(MSGFMT) --desktop --template $< -d $(top_srcdir)/po -o $@

@GSETTINGS_RULES@
