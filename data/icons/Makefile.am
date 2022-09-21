icondir = $(datadir)/icons/hicolor/scalable/apps
icon_DATA = scalable/org.gtkhash.gtkhash.svg

icon32dir = $(datadir)/icons/hicolor/32x32/apps
icon32_DATA = 32/org.gtkhash.gtkhash.png

icon48dir = $(datadir)/icons/hicolor/48x48/apps
icon48_DATA = 48/org.gtkhash.gtkhash.png

icon64dir = $(datadir)/icons/hicolor/64x64/apps
icon64_DATA = 64/org.gtkhash.gtkhash.png

icon128dir = $(datadir)/icons/hicolor/128x128/apps
icon128_DATA = 128/org.gtkhash.gtkhash.png

icon256dir = $(datadir)/icons/hicolor/256x256/apps
icon256_DATA = 256/org.gtkhash.gtkhash.png

genicons = \
	$(icon32_DATA) \
	$(icon48_DATA) \
	$(icon64_DATA) \
	$(icon128_DATA) \
	$(icon256_DATA)

EXTRA_DIST = \
	$(icon_DATA) \
	$(genicons) \
	meson.build \
	rename_icons.sh

MAINTAINERCLEANFILES = $(genicons)

%.png: $(icon_DATA)
	$(AM_V_GEN) $(MKDIR_P) -- $(@D) && \
		rsvg-convert -w $(@D) -h $(@D) $< -o $@
