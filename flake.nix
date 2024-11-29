# run with:                nix develop
# see metadata with:       nix flake metadata
# debug with:              nix repl
#                          :lf .#
# check with:              nix flake check
# If you want to update a locked input to the latest version, you need to ask
# for it:                  nix flake lock --update-input nixpkgs
# show available packages: nix-env -qa
#                          nix search nixpkgs
# show nixos version:      nixos-version
# 

{
  description = "C++ Development with Nix 24.11";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-24.11";
    nixpkgs-unstable.url = "github:nixos/nixpkgs/nixpkgs-unstable";
  };

  outputs = { self, nixpkgs, nixpkgs-unstable }: {
    devShells = {
      x86_64-linux.default  = self.buildDevShell "x86_64-linux";
      aarch64-linux.default = self.buildDevShell "aarch64-linux";
      x86_64-darwin.default = self.buildDevShell "x86_64-darwin";
    };
  } // {
    buildDevShell = system: let
      pkgs = import nixpkgs { inherit system; };
      pkgsUnstable = import nixpkgs-unstable { inherit system; };
    in
      pkgs.mkShell {
        packages = with pkgs; [
          gcc14Stdenv
          gdb
          # coreutils
          # gnumake
          # ninja
        ];
        buildInputs = with pkgsUnstable; [
          qt6.full
          qt6.qtbase
          qt6.qtdoc
          qt6.qtsvg
          qt6.qtquick3d
          qt6.qtwebengine
          qt6.qtwayland
          qt6.qtserialport
          qt6.qtshadertools
          qt6.qt5compat
          qt6.qtdeclarative
          qt6.qtquicktimeline
          # qt6.qtbase
          # qt6.qtquick3d
          # qt6.qtdeclarative
          openssl
          # qt6.wrapQtAppsHook
        ];
        nativeBuildInputs = with pkgsUnstable;  [
          cmake
          pkg-config
          qt6.qttools
          qt6.wrapQtAppsHook
        ];
        # QT_PLUGIN_PATH = "${qtbase}/${qtbase.qtPluginPrefix}";


        # # set the environment variables that Qt apps expect
        # shellHook = ''
        #   bashdir=$(mktemp -d)
        #   makeWrapper "$(type -p bash)" "$bashdir/bash" "''${qtWrapperArgs[@]}"
        #   exec "$bashdir/bash"
        # '';
      };
  };
}
