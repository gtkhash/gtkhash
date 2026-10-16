#!/usr/bin/env bash
set -e

APP_ID="org.gtkhash.gtkhash"
T=$(mktemp -d)

export GIO_USE_VFS="local"
export GSETTINGS_BACKEND="memory"
export LC_ALL="C.utf8"
export SWAYSOCK="${T}/sway.sock"
export WAYLAND_DISPLAY="wayland-1"
export WLR_BACKENDS="headless"
export WLR_RENDERER="pixman"
export XDG_RUNTIME_DIR=${T}

cleanup() {
	kill $(jobs -p) 2>/dev/null || :
	rm -rf "${T}"
}
trap cleanup EXIT

ISO="${T}/example.iso"
genisoimage -quiet -output "${ISO}" /var/empty
sha256sum "${ISO}" > "${ISO}.sha256"

{
	sha256sum /etc/{group,hosts,issue}
	echo "aaaaaaaa  /etc/profile"
	sha256sum /etc/{services,shells}
} > "${T}/etc.sha256"

start_sway() {
	sway --config "${T}/sway.config" &
	until swaymsg -t get_tree 2>/dev/null | grep -q "${APP_ID}" ; do
		sleep 1
	done
}

stop_sway() {
	swaymsg exit 2>/dev/null || :
	rm -f "${SWAYSOCK}"
}

screenshot() {
	local out=$1
	shift
	local black_bg="${T}/black-bg.png"
	local white_bg="${T}/white-bg.png"
	local resize=

	if [[ -n ${WINDOW_SIZE} ]] ; then
		resize=", resize set ${WINDOW_SIZE}"
	fi

	cat > "${T}/sway.config" <<-EOF
		output HEADLESS-1 res 1920x1080 scale 2
		default_border none
		xwayland disable
		for_window [app_id="${APP_ID}"] floating enable${resize}
		exec meson devenv -C _build src/gtkhash $@
	EOF

	start_sway

	swaymsg "output HEADLESS-1 bg #000000 solid_color"
	grim "${black_bg}"

	swaymsg "output HEADLESS-1 bg #ffffff solid_color"
	grim "${white_bg}"

	stop_sway

	magick "${black_bg}" "${white_bg}" \
		-verbose \
		-alpha off \
		\( -clone 0,1 -compose difference -composite -negate \) \
		\( -clone 0,2 -compose divide -composite \) \
		-delete 0,1 \
		+swap -compose CopyOpacity -composite \
		-trim +repage \
		-strip -define png:compression-level=9 \
		"${out}"
}

GTK_THEME="Adwaita" \
	screenshot screenshots/gtkhash-screenshot-light.png \
	-f BLAKE3 -C "${ISO}.sha256"
GTK_THEME="Adwaita:dark" \
	screenshot screenshots/gtkhash-screenshot-dark.png \
	-f BLAKE3 -C "${ISO}.sha256"
GTK_THEME="Adwaita" WINDOW_SIZE="600 400" \
	screenshot screenshots/file-list.png \
	-f -MD5 -f -SHA1 -f -CRC32 -C "${T}/etc.sha256"
