{
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }: {
    packages = nixpkgs.lib.genAttrs nixpkgs.lib.systems.flakeExposed (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in rec {
        gtkhash = pkgs.clangStdenv.mkDerivation {
          name = "gtkhash";
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
            libgcrypt
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
            mainProgram = "gtkhash";
            license = pkgs.lib.licenses.gpl2Plus;
          };
        };
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
