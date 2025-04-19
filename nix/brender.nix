{ stdenv
, lib
, version
, cmake
, perl
, uasm
, wineserverHook
, SDL2
, glfw
, libGL
, makeBinaryWrapper
, darwin
, clang-tools
, withTools ? true
, withExamples ? false
}: let
  binExtension = stdenv.hostPlatform.extensions.executable;
in
stdenv.mkDerivation(finalAttrs: {
  inherit version;

  pname = "brender";

  src = ../.;

  nativeBuildInputs = [
    cmake
    perl
    makeBinaryWrapper
  ] ++ lib.optionals stdenv.isx86_32 [
    uasm
    wineserverHook
  ];

  passthru.devTools = [
    clang-tools
  ];

  buildInputs = [
    SDL2
    libGL
  ] ++ lib.optionals stdenv.isDarwin [
    # Why doesn't SDL pull this in?
    darwin.apple_sdk.frameworks.OpenGL
  ] ++ lib.optionals withExamples [
    glfw
  ];

  cmakeFlags = [
    (lib.cmakeBool "BRENDER_BUILD_TOOLS" withTools)
    (lib.cmakeBool "BRENDER_BUILD_EXAMPLES" withExamples)
    (lib.cmakeBool "BRENDER_BUILD_GLFW_EXAMPLE" withExamples)
    (lib.cmakeBool "BRENDER_BUILD_SOFT" stdenv.isx86_32)
  ] ++ lib.optionals stdenv.isx86_32 [
    "-DCMAKE_ASM_MASM_COMPILER=uasm"
  ];

  postFixup = ''
    mkdir -p $out
  '' + lib.optionalString withExamples ''
    mkdir -p $data/share/brender-samples/{robot,brintro,dat,tutorials}

    cp ${finalAttrs.src}/examples/robot/dat/*   $data/share/brender-samples/robot
    cp ${finalAttrs.src}/examples/brintro/dat/* $data/share/brender-samples/brintro
    cp ${finalAttrs.src}/examples/dat/*         $data/share/brender-samples/dat

    wrapProgram $bin/bin/robot${binExtension} \
      --set BRENDER_PATH $data/share/brender-samples/robot

    wrapProgram $bin/bin/cube${binExtension} \
      --set BRENDER_PATH $data/share/brender-samples/dat

    wrapProgram $bin/bin/devpmtest${binExtension} \
      --set BRENDER_PATH $data/share/brender-samples/dat

    wrapProgram $bin/bin/brintro${binExtension} \
      --set BRENDER_PATH $data/share/brender-samples/brintro


    cp ${finalAttrs.src}/examples/tutorials/dat/* $data/share/brender-samples/tutorials

    for i in {1,2,3,4,5,6,7,8,9,10}; do
      wrapProgram $bin/bin/brtutor$i${binExtension} \
        --set BRENDER_PATH $data/share/brender-samples/tutorials
    done

    for i in minzb minzs sheet rdemo forest; do
      if  [[ -e ${finalAttrs.src}/examples/samples/$i/dat ]]; then
        mkdir -p $data/share/brender-samples/samples/$i
        cp ${finalAttrs.src}/examples/samples/$i/dat/* $data/share/brender-samples/samples/$i
        wrapProgram $bin/bin/$i${binExtension} \
          --set BRENDER_PATH $data/share/brender-samples/samples/$i
      fi
    done
  '';

  outputs = [ "bin" "lib" "dev" "out" ] ++ (lib.optionals withExamples [
    "data"
  ]);
})
