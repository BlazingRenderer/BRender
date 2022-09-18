{ stdenv, version, cmake, perl, SDL2, makeWrapper }:
stdenv.mkDerivation rec {
  inherit version;

  pname = "brender-samples";

  src = ./.;

  nativeBuildInputs = [
    cmake
    perl
    SDL2
    makeWrapper
  ];

  installPhase = ''
    mkdir -p $out/{bin,share/brender-samples/robot}
    cp tools/mkranges/mkranges tools/mkshades/mkshades examples/robot/robot $out/bin
    cp ${src}/examples/robot/dat/* $out/share/brender-samples/robot

    wrapProgram $out/bin/robot \
      --chdir $out/share/brender-samples/robot
  '';
}
