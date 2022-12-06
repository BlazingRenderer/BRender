{
  description = "BRender";

  outputs = { self, nixpkgs, ... }: let
    mkPackages = { pkgs }: rec {
      brender-samples = pkgs.callPackage ./default.nix {
        version = self.lastModifiedDate;
      };

      default = brender-samples;
    };

    mkShells = packages: builtins.mapAttrs (k: v: v.overrideAttrs(old: {
      hardeningDisable = [ "all" ];
    })) packages;
  in {
    packages.x86_64-linux   = mkPackages { pkgs = import nixpkgs { system = "x86_64-linux";   }; };
    packages.x86_64-darwin  = mkPackages { pkgs = import nixpkgs { system = "x86_64-darwin";  }; };
    packages.aarch64-darwin = mkPackages { pkgs = import nixpkgs { system = "aarch64-darwin"; }; };

    devShells.x86_64-linux   = mkShells self.packages.x86_64-linux;
    devShells.x86_64-darwin  = mkShells self.packages.x86_64-darwin;
    devShells.aarch64-darwin = mkShells self.packages.x86_64-darwin;
  };
}
