{
  description = "BRender";

  inputs.nixpkgs.url = github:NixOS/nixpkgs;

  outputs = { self, nixpkgs, ... }: let
    forAllSystems = function:
        nixpkgs.lib.genAttrs [
          "x86_64-linux"
          "aarch64-linux"
          "x86_64-darwin"
          "aarch64-darwin"
        ] (system: function nixpkgs.legacyPackages.${system});

    mkShells = packages: builtins.mapAttrs (k: v: v.overrideAttrs(old: {
      hardeningDisable = [ "all" ];
      nativeBuildInputs = old.nativeBuildInputs ++ v.devTools;
    })) (nixpkgs.lib.removeAttrs packages [ "wineserverHook" "uasm" "uasm-static" ]);
  in {
    packages = forAllSystems(pkgs: rec {
      wineserverHook = pkgs.callPackage ./nix/wineserver-hook.nix { };

      uasm = pkgs.callPackage ./nix/uasm { };

      uasm-static = pkgs.pkgsStatic.callPackage ./nix/uasm { };

      brender = pkgs.callPackage ./nix/brender.nix {
        inherit wineserverHook uasm;

        version = self.lastModifiedDate;
      };

      brender-samples = brender.override { withExamples = true; };

      brender-samples-clang = brender-samples.override { stdenv = pkgs.clangStdenv; };

      default = brender;

      brender-samples-win32 = pkgs.pkgsCross.mingw32.callPackage ./nix/brender.nix {
        inherit wineserverHook uasm;

        version = self.lastModifiedDate;
        withExamples = true;
      };

      brender-samples-win64 = pkgs.pkgsCross.mingwW64.callPackage ./nix/brender.nix {
        inherit wineserverHook uasm;

        version = self.lastModifiedDate;
        withExamples = true;
      };

      brender-samples-linux32 = pkgs.pkgsi686Linux.callPackage ./nix/brender.nix {
        inherit wineserverHook uasm;

        version = self.lastModifiedDate;
        withExamples = true;
      };
    });

    devShells.x86_64-linux   = mkShells self.packages.x86_64-linux;
    devShells.aarch64-linux  = mkShells self.packages.aarch64-linux;
    devShells.x86_64-darwin  = mkShells self.packages.x86_64-darwin;
    devShells.aarch64-darwin = mkShells self.packages.aarch64-darwin;
  };
}
