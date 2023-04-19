{ stdenv, version, cmake, perl, SDL2, makeBinaryWrapper }:
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

  buildInputs = [
    SDL2
  ];

  outputs = [ "bin" "out" ];
}
