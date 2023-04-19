{ stdenv, version, cmake, perl, SDL2, makeBinaryWrapper, clang-tools }:
let
  binExtension = "${stdenv.hostPlatform.extensions.executable}";
in
stdenv.mkDerivation rec {
  inherit version;

  pname = "brender-samples";

  src = ./.;

  nativeBuildInputs = [
    cmake
    perl
    makeBinaryWrapper
  ];

  passthru.SDL2 = SDL2; # 'cause it's so damned hard to debug with CMake

  passthru.devTools = [
    clang-tools
  ];

  buildInputs = [
    SDL2
  ];

  installPhase = ''
    mkdir -p $out/{bin,share/brender-samples/{robot,dat,tutorials}}

    for i in 3ds2br dquery mkblend mkfog mkranges mkshades paljoin texconv texview matconv; do
      cp tools/$i/$i${binExtension} $out/bin
    done

    for i in mkres robot cube devpmtest ddrawtest; do
      if [[ -e examples/$i/$i${binExtension} ]]; then
        cp examples/$i/$i${binExtension} $out/bin
      fi
    done

    cp ${src}/examples/robot/dat/* $out/share/brender-samples/robot
    cp ${src}/examples/dat/* $out/share/brender-samples/dat

    wrapProgram $out/bin/robot${binExtension} \
      --chdir $out/share/brender-samples/robot

    wrapProgram $out/bin/cube${binExtension} \
      --set BRENDER_PATH $out/share/brender-samples/dat

    wrapProgram $out/bin/devpmtest${binExtension} \
      --set BRENDER_PATH $out/share/brender-samples/dat


    cp ${src}/examples/tutorials/dat/* $out/share/brender-samples/tutorials

    for i in {1,2,3,4,5,5b,6,6b,7,7b,8,8b,9,10}; do
      cp examples/tutorials/brtutor$i${binExtension} $out/bin

      wrapProgram $out/bin/brtutor$i${binExtension} \
        --chdir $out/share/brender-samples/tutorials
    done
  '';
}
