{ stdenv, version, cmake, perl, SDL2, makeBinaryWrapper, clang-tools }:
let
  binExtension = "${stdenv.hostPlatform.extensions.executable}";
in
stdenv.mkDerivation rec {
  inherit version;

  pname = "brender";

  src = ./.;

  nativeBuildInputs = [
    cmake
    perl
    makeBinaryWrapper
  ];

  passthru.devTools = [
    clang-tools
  ];

  buildInputs = [
    SDL2
  ];

  outputs = [ "bin" "out" ];
}
