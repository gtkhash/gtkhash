nautilusdir = $(pkgdatadir)/nautilus
nautilus_DATA = gtkhash-properties.xml

schemadir = $(GCONF_SCHEMA_FILE_DIR)
schema_DATA = gtkhash.schemas

EXTRA_DIST = gtkhash-properties.xml gtkhash.schemas

if GCONF_SCHEMAS_INSTALL
install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) \
		--makefile-install-rule $(schema_DATA)
else
install-data-local:
endif
