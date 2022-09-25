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
    mkdir -p $out/{bin,share/brender-samples/{robot,dat}}
    cp tools/3ds2br/3ds2br $out/bin
    cp tools/mkranges/mkranges $out/bin
    cp tools/mkshades/mkshades $out/bin
    cp examples/robot/robot $out/bin
    cp examples/cube/cube $out/bin

    cp ${src}/examples/robot/dat/* $out/share/brender-samples/robot
    cp ${src}/examples/dat/* $out/share/brender-samples/dat

    wrapProgram $out/bin/robot \
      --chdir $out/share/brender-samples/robot

    wrapProgram $out/bin/cube \
      --set BRENDER_PATH $out/share/brender-samples/dat
  '';
}
