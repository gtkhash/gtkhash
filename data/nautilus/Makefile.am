nautilusdir = $(pkgdatadir)/nautilus
nautilus_DATA = gtkhash-properties.xml.gz

schemadir = $(GCONF_SCHEMA_FILE_DIR)
schema_DATA = gtkhash.schemas

EXTRA_DIST = gtkhash-properties.xml gtkhash-properties.xml.gz gtkhash.schemas
CLEANFILES = $(nautilus_DATA)

gtkhash-properties.xml.gz: gtkhash-properties.xml
	gzip -c9 $< > $@

if GCONF_SCHEMAS_INSTALL
install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) \
		--makefile-install-rule $(srcdir)/$(schema_DATA)
else
install-data-local:
endif
