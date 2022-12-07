{ stdenv, version, cmake, perl, SDL2, makeBinaryWrapper }:
stdenv.mkDerivation rec {
  inherit version;

  pname = "brender-samples";

  src = ./.;

  nativeBuildInputs = [
    cmake
    perl
    makeBinaryWrapper
  ];

  buildInputs = [
    SDL2
  ];

  installPhase = ''
    mkdir -p $out/{bin,share/brender-samples/{robot,dat,tutorials}}
    cp tools/3ds2br/3ds2br $out/bin
    cp tools/dquery/dquery $out/bin
    cp tools/mkblend/mkblend $out/bin
    cp tools/mkfog/mkfog $out/bin
    cp tools/mkranges/mkranges $out/bin
    cp tools/mkshades/mkshades $out/bin
    cp tools/paljoin/paljoin $out/bin
    cp tools/texconv/texconv $out/bin
    cp tools/matconv/matconv $out/bin
    cp examples/mkres/mkres $out/bin
    cp examples/robot/robot $out/bin
    cp examples/cube/cube $out/bin
    cp examples/devpmtest/devpmtest $out/bin

    cp ${src}/examples/robot/dat/* $out/share/brender-samples/robot
    cp ${src}/examples/dat/* $out/share/brender-samples/dat

    wrapProgram $out/bin/robot \
      --chdir $out/share/brender-samples/robot

    wrapProgram $out/bin/cube \
      --set BRENDER_PATH $out/share/brender-samples/dat

    wrapProgram $out/bin/devpmtest \
      --set BRENDER_PATH $out/share/brender-samples/dat


    cp ${src}/examples/tutorials/dat/* $out/share/brender-samples/tutorials

    for i in {1,2,3,4,5,5b,6,6b,7,7b,8,8b,9,10}; do
      cp examples/tutorials/brtutor$i $out/bin

      wrapProgram $out/bin/brtutor$i \
        --chdir $out/share/brender-samples/tutorials
    done
  '';
}
