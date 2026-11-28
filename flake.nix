{
  description = "BRender";

  inputs.nixpkgs.url = "nixpkgs";

  outputs = { self, nixpkgs, ... }: let
    lib = nixpkgs.lib;

    forAllSystems = function:
        nixpkgs.lib.genAttrs [
          "x86_64-linux"
          "aarch64-linux"
          "aarch64-darwin"
        ] (system: function nixpkgs.legacyPackages.${system});

    mkShells = let
      mapPkgs = builtins.mapAttrs (k: v: v.overrideAttrs(old: {
        hardeningDisable = [ "all" ];
        nativeBuildInputs = old.nativeBuildInputs ++ v.devTools;

        env.LD_LIBRARY_PATH = self.inputs.nixpkgs.lib.makeLibraryPath [ v.vulkan-loader ];
      }));
    in packages: (
      (mapPkgs packages) //
      (lib.optionalAttrs packages.brender.stdenv.hostPlatform.isLinux ({
        rendertest = packages.brender.override {
          withTools = true;
          useGlRuntime = true;
        };
      })));
  in {
    packages = forAllSystems(pkgs: (rec {
      brender = pkgs.callPackage ./nix/brender.nix {
        version = self.lastModifiedDate;
      };

      brender-samples = brender.override { withExamples = true; };

      brender-samples-clang = brender-samples.override { stdenv = pkgs.clangStdenv; };

      default = brender;

      brender-samples-win32 = pkgs.pkgsCross.mingw-msvcrt-i686.callPackage ./nix/brender.nix {
        version = self.lastModifiedDate;
        withExamples = true;
      };

      brender-samples-win64 = pkgs.pkgsCross.mingw-msvcrt-x86_64.callPackage ./nix/brender.nix {
        version = self.lastModifiedDate;
        withExamples = true;
      };

      brender-dos-i686 = pkgs.callPackage ./nix/dos.nix {
        version = self.lastModifiedDate;

        systemProcessor = "i686";
        djgpp = pkgs.djgpp_i686;
        djgppPrefix = "i686-pc-msdosdjgpp-";
      };

      brender-dos-i586 = pkgs.callPackage ./nix/dos.nix {
        version = self.lastModifiedDate;

        systemProcessor = "i586";
        djgpp = pkgs.djgpp_i586;
        djgppPrefix = "i586-pc-msdosdjgpp-";
      };
    }) // (lib.optionalAttrs pkgs.stdenv.hostPlatform.isx86 {

      brender-samples-linux32 = pkgs.pkgsi686Linux.callPackage ./nix/brender.nix {
        inherit (pkgs) cmake perl spirv-cross glslang; # It appears pkgsi686Linux doesn't splice.

       version = self.lastModifiedDate;
       withExamples = true;
      };
    }));

    # `nix flake check` builds these, and CI runs it, so formatting is gated where
    # the rest of the tree's checks already are. clang-tools comes from the flake's
    # own nixpkgs, so a formatter upgrade reformats the tree in one visible change
    # instead of reddening an unrelated diff.
    checks = forAllSystems (pkgs: {
      format = pkgs.runCommandLocal "format" {
        nativeBuildInputs = [ pkgs.clang-tools ];
      } ''
        cd ${self}
        find core drivers tools examples contrib \
          \( -name '*.h' -o -name '*.c' -o -name '*.cpp' -o -name '*.hpp' \) -print0 \
          | xargs -0 -r clang-format --dry-run --Werror > $out
      '';
    });

    devShells.x86_64-linux   = mkShells self.packages.x86_64-linux;
    devShells.aarch64-linux  = mkShells self.packages.aarch64-linux;
    devShells.aarch64-darwin = mkShells self.packages.aarch64-darwin;
  };
}
