{
  description = "BRender";

  outputs = { self, nixpkgs, ... }: let
    mkPackages = { system }: let
      pkgs = import nixpkgs { inherit system; };
    in rec {
      brender-samples = pkgs.callPackage ./default.nix {
        version = self.lastModifiedDate;
      };

      brender-samples-clang = brender-samples.override { stdenv = pkgs.clangStdenv; };

      default = brender-samples;
    };

    mkCross = { crossSystem }: let
      crossPackages = import nixpkgs {
        inherit crossSystem;
        system = "x86_64-linux";
      };
    in crossPackages.callPackage ./default.nix { version = self.lastModifiedDate; };

    mkShells = packages: builtins.mapAttrs (k: v: v.overrideAttrs(old: {
      hardeningDisable = [ "all" ];
    })) packages;
  in {
    packages.x86_64-linux = (mkPackages { system = "x86_64-linux"; }) // {
      brender-samples-win32 = mkCross { crossSystem = nixpkgs.lib.systems.examples.mingw32;  };
      brender-samples-win64 = mkCross { crossSystem = nixpkgs.lib.systems.examples.mingwW64; };
    };

    packages.x86_64-darwin  = mkPackages { system = "x86_64-darwin"; };
    packages.aarch64-darwin = mkPackages { system = "aarch64-darwin"; };

    devShells.x86_64-linux   = mkShells self.packages.x86_64-linux;
    devShells.x86_64-darwin  = mkShells self.packages.x86_64-darwin;
    devShells.aarch64-darwin = mkShells self.packages.x86_64-darwin;
  };
}
