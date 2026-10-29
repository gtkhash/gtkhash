{
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }: {
    packages = nixpkgs.lib.genAttrs nixpkgs.lib.systems.flakeExposed (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        common = {
          src = self.sourceInfo;
          strictDeps = true;
          doCheck = true;
          mesonBuildType = "debugoptimized";
          nativeBuildInputs = with pkgs; [
            appstream
            gettext
            libxml2 # xmllint
            meson
            ninja
            pkg-config
          ];
          buildInputs = with pkgs; [
            libb2
            libblake3
            libgcrypt
            xxhash
            zlib
          ];
          nativeCheckInputs = with pkgs; [
            hicolor-icon-theme
            xvfb-run
          ];
          preConfigure = ''
            # Required for LTO
            export AR="gcc-ar"
          '';
          checkPhase = ''
            runHook preCheck
            HOME=$(mktemp -d) \
            XDG_DATA_DIRS+=":${pkgs.glib.getSchemaDataDirPath pkgs.gtk3}" \
              xvfb-run --auto-servernum -s "-screen 0 800x600x24" meson test -v
            runHook postCheck
          '';
          meta = {
            license = pkgs.lib.licenses.gpl2Plus;
          };
        };
      in rec {
        gtkhash = pkgs.stdenv.mkDerivation (common // {
          name = "gtkhash";
          nativeBuildInputs = common.nativeBuildInputs ++ (with pkgs; [
            desktop-file-utils
            gtk3 # gtk-update-icon-cache
            wrapGAppsHook3
          ]);
          buildInputs = common.buildInputs ++ (with pkgs; [
            gtk3
            librsvg # libpixbufloader_svg
          ]);
          meta = common.meta // {
            mainProgram = "gtkhash";
          };
        });

        gtkhash-caja = pkgs.stdenv.mkDerivation (common // {
          name = "gtkhash-caja";
          mesonFlags = [
            "-Dbuild-gtkhash=false"
            "-Dbuild-caja=true"
          ];
          nativeBuildInputs = common.nativeBuildInputs ++ (with pkgs; [
            gtk3 # gtk-update-icon-cache
          ]);
          buildInputs = common.buildInputs ++ (with pkgs; [
            caja
            gtk3
          ]);
          env.PKG_CONFIG_LIBCAJA_EXTENSION_EXTENSIONDIR =
            "${placeholder "out"}/lib/caja/extensions-2.0";
        });

        gtkhash-nautilus = pkgs.stdenv.mkDerivation (common // {
          name = "gtkhash-nautilus";
          mesonFlags = [
            "-Dbuild-gtkhash=false"
            "-Dbuild-nautilus=true"
          ];
          buildInputs = common.buildInputs ++ (with pkgs; [
            nautilus
          ]);
          env.PKG_CONFIG_LIBNAUTILUS_EXTENSION_4_EXTENSIONDIR =
            "${placeholder "out"}/lib/nautilus/extensions-4";
        });

        gtkhash-nemo = pkgs.stdenv.mkDerivation (common // {
          name = "gtkhash-nemo";
          mesonFlags = [
            "-Dbuild-gtkhash=false"
            "-Dbuild-nemo=true"
          ];
          buildInputs = common.buildInputs ++ (with pkgs; [
            gtk3
            nemo
          ]);
          env.PKG_CONFIG_LIBNEMO_EXTENSION_EXTENSIONDIR =
            "${placeholder "out"}/${pkgs.nemo.extensiondir}";
        });

        gtkhash-thunar = pkgs.stdenv.mkDerivation (common // {
          name = "gtkhash-thunar";
          mesonFlags = [
            "-Dbuild-gtkhash=false"
            "-Dbuild-thunar=true"
          ];
          buildInputs = common.buildInputs ++ (with pkgs; [
            gtk3
            thunar
          ]);
          env.PKG_CONFIG_THUNARX_3_EXTENSIONSDIR =
            "${placeholder "out"}/lib/thunarx-3";
        });

        default = gtkhash;
      }
    );

    devShells = nixpkgs.lib.genAttrs nixpkgs.lib.systems.flakeExposed (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in {
        default = pkgs.mkShell {
          packages = with pkgs; [
            appstream
            caja
            cdrkit # genisoimage
            desktop-file-utils
            gettext
            grim
            gtk3
            imagemagick # magick
            libb2
            libblake3
            libgcrypt
            librsvg # libpixbufloader_svg
            libxml2 # xmllint
            mbedtls
            meson
            nautilus
            nemo
            nettle
            ninja
            openssl
            pkg-config
            sway
            thunar
            xvfb-run
            xxhash
            zlib
          ];
          shellHook = ''
            export AR="gcc-ar"
            export XDG_DATA_DIRS+=":${pkgs.glib.getSchemaDataDirPath pkgs.gtk3}"
          '';
        };
      }
    );
  };
}
