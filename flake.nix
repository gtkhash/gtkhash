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
            desktop-file-utils
            gtk3
            librsvg
            meson
            ninja
            pkg-config
            wrapGAppsHook3
          ];
          buildInputs = with pkgs; [
            gtk3
            libb2
            libblake3
            libgcrypt
            xxhash
          ];
          nativeCheckInputs = with pkgs; [
            hicolor-icon-theme
            xvfb-run
          ];
          preConfigure = ''
            # Required for LTO
            export AR="${pkgs.llvm}/bin/llvm-ar"
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
        gtkhash = pkgs.clangStdenv.mkDerivation (common // {
          name = "gtkhash";
          meta = common.meta // {
            mainProgram = "gtkhash";
          };
        });
        gtkhash-caja = pkgs.clangStdenv.mkDerivation (common // {
          name = "gtkhash-caja";
          mesonFlags = [
            "-Dbuild-gtkhash=false"
            "-Dbuild-caja=true"
          ];
          buildInputs = common.buildInputs ++ [ pkgs.caja ];
          env.PKG_CONFIG_LIBCAJA_EXTENSION_EXTENSIONDIR =
            "${placeholder "out"}/lib/caja/extensions-2.0";
        });
        gtkhash-nemo = pkgs.clangStdenv.mkDerivation (common // {
          name = "gtkhash-nemo";
          mesonFlags = [
            "-Dbuild-gtkhash=false"
            "-Dbuild-nemo=true"
          ];
          buildInputs = common.buildInputs ++ [ pkgs.nemo ];
          env.PKG_CONFIG_LIBNEMO_EXTENSION_EXTENSIONDIR =
            "${placeholder "out"}/${pkgs.nemo.extensiondir}";
        });
        gtkhash-thunar = pkgs.clangStdenv.mkDerivation (common // {
          name = "gtkhash-thunar";
          mesonFlags = [
            "-Dbuild-gtkhash=false"
            "-Dbuild-thunar=true"
          ];
          buildInputs = common.buildInputs ++ [ pkgs.thunar ];
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
        default = pkgs.mkShell.override { stdenv = pkgs.clangStdenv; } {
          packages = with pkgs; [
            caja
            desktop-file-utils
            gettext
            gtk3
            libb2
            libblake3
            libgcrypt
            librsvg # rsvg-convert
            libxml2 # xmllint
            mbedtls
            meson
            nautilus
            nemo
            nettle
            ninja
            openssl
            pkg-config
            thunar
            xvfb-run
            xxhash
          ];
          shellHook = ''
            export AR="${pkgs.llvm}/bin/llvm-ar"
            export XDG_DATA_DIRS+=":${pkgs.glib.getSchemaDataDirPath pkgs.gtk3}"
          '';
        };
      }
    );
  };
}
