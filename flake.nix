{
  description = "BRender";

  inputs.nixpkgs.url = github:NixOS/nixpkgs;

  outputs = { self, nixpkgs, ... }: let
    mkPackages = { system }: let
      pkgs = import nixpkgs { inherit system; };
    in rec {
      brender = pkgs.callPackage ./default.nix {
        version = self.lastModifiedDate;
      };

      brender-samples = brender.override { withExamples = true; };

      brender-samples-clang = brender-samples.override { stdenv = pkgs.clangStdenv; };

      default = brender;
    };

    mkCross = { crossSystem }: let
      crossPackages = import nixpkgs {
        inherit crossSystem;
        system = "x86_64-linux";
      };
    in crossPackages.callPackage ./default.nix {
      version = self.lastModifiedDate;

      SDL2 = crossPackages.SDL2.override {
        withStatic = crossPackages.hostPlatform.isWindows;
      };
    };

    mkShells = packages: builtins.mapAttrs (k: v: v.overrideAttrs(old: {
      hardeningDisable = [ "all" ];
      nativeBuildInputs = old.nativeBuildInputs ++ v.devTools;
    })) packages;
  in {
    packages.x86_64-linux = (mkPackages { system = "x86_64-linux"; }) // {
      brender-samples-win32 = mkCross { crossSystem = nixpkgs.lib.systems.examples.mingw32;  };
      brender-samples-win64 = mkCross { crossSystem = nixpkgs.lib.systems.examples.mingwW64; };

      brender-samples-lin32 = nixpkgs.legacyPackages.x86_64-linux.pkgsi686Linux.callPackage ./default.nix {
        version = self.lastModifiedDate;
      };
    };

    packages.i686-linux     = mkPackages { system = "i686-linux"; };
    packages.x86_64-darwin  = mkPackages { system = "x86_64-darwin"; };
    packages.aarch64-linux  = mkPackages { system = "aarch64-linux"; };
    packages.aarch64-darwin = mkPackages { system = "aarch64-darwin"; };

    devShells.x86_64-linux   = mkShells self.packages.x86_64-linux;
    devShells.i686-linux     = mkShells self.packages.i686-linux;
    devShells.aarch64-linux  = mkShells self.packages.aarch64-linux;
    devShells.x86_64-darwin  = mkShells self.packages.x86_64-darwin;
    devShells.aarch64-darwin = mkShells self.packages.aarch64-darwin;
  };
}
