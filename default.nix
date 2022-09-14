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
    cp mkranges/mkranges mkshades/mkshades robot/robot $out/bin
    cp ${src}/robot/dat/* $out/share/brender-samples/robot

    wrapProgram $out/bin/robot \
      --chdir $out/share/brender-samples/robot
  '';
}
