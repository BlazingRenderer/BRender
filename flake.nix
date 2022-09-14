{
  description = "BRender";

  outputs = { self, nixpkgs, ... }: {
    packages.x86_64-linux = let
      pkgs = import nixpkgs { system = "x86_64-linux"; };
    in rec {
      brender-samples = pkgs.callPackage ./default.nix {
        version = self.lastModifiedDate;
      };

      default = brender-samples;
    };
  };
}