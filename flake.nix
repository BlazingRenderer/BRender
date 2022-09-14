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
  };
}
