{
  description = "BRender";

  outputs = { self, nixpkgs, ... }: let
    mkPackages = { pkgs }: rec {
      brender-samples = pkgs.callPackage ./default.nix {
        version = self.lastModifiedDate;
      };

      default = brender-samples;
    };
  in {
    packages.x86_64-linux   = mkPackages { pkgs = import nixpkgs { system = "x86_64-linux";   }; };
    packages.x86_64-darwin  = mkPackages { pkgs = import nixpkgs { system = "x86_64-darwin";  }; };
    packages.aarch64-darwin = mkPackages { pkgs = import nixpkgs { system = "aarch64-darwin"; }; };

    devShells.x86_64-linux.default = self.packages.x86_64-linux.default.overrideAttrs(old: {
      hardeningDisable = [ "all" ];
    });

    devShells.x86_64-darwin.default = self.packages.x86_64-darwin.default.overrideAttrs(old: {
      hardeningDisable = [ "all" ];
    });

    devShells.aarch64-darwin.default = self.packages.aarch64-darwin.default.overrideAttrs(old: {
      hardeningDisable = [ "all" ];
    });
  };
}
