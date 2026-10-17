{ stdenv
, lib
, version
, cmake
, perl
, sdl3
, glfw
, glslang
, spirv-cross
, makeBinaryWrapper
, mesa
, libglvnd
, clang-tools
, withTools ? true
, withExamples ? false
}: let
  binExtension = stdenv.hostPlatform.extensions.executable;

  runTests = withExamples && stdenv.buildPlatform.canExecute stdenv.hostPlatform;

  # The GL tests ask SDL for an OpenGL window. A sandbox has no GPU and, unlike
  # a devshell, no system EGL vendor config, so SDL's EGL cannot find a device
  # and the demo falls back to the software renderer while still labelling the
  # run `device=glrend`. On Linux, mesa + libglvnd supply llvmpipe for it; on
  # darwin and the other platforms GL comes from the system and no vendoring is
  # needed.
  glRuntime = withExamples && stdenv.hostPlatform.isLinux;
in
stdenv.mkDerivation(finalAttrs: {
  inherit version;

  pname = "brender";

  src = ../.;

  nativeBuildInputs = [
    cmake
    perl
    makeBinaryWrapper
    glslang
    spirv-cross
  ];

  passthru.devTools = [
    clang-tools
  ];

  buildInputs = [
    sdl3
  ] ++ lib.optionals withExamples [
    glfw
  ];

  checkInputs = lib.optionals glRuntime [
    mesa
    libglvnd
  ];

  # Point the GL tests at llvmpipe. SDL dlopens libEGL by soname and the mesa
  # EGL vendor lives in the mesa store path, so the loader needs both on the
  # search path; the vendor JSON names the vendor library explicitly. The
  # per-test LIBGL_ALWAYS_SOFTWARE the corpus sets is not enough on its own -
  # without a device to initialise, EGL never reaches the software driver.
  preCheck = lib.optionalString glRuntime ''
    export LIBGL_ALWAYS_SOFTWARE=1
    export GALLIUM_DRIVER=llvmpipe
    export LIBGL_DRIVERS_PATH=${mesa}/lib/dri
    export __EGL_VENDOR_LIBRARY_FILENAMES=${mesa}/share/glvnd/egl_vendor.d/50_mesa.json
    export LD_LIBRARY_PATH=${libglvnd}/lib:${mesa}/lib''${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
  '';

  cmakeFlags = [
    (lib.cmakeBool "BRENDER_BUILD_TOOLS" withTools)
    (lib.cmakeBool "BRENDER_BUILD_EXAMPLES" withExamples)
    (lib.cmakeBool "BRENDER_BUILD_GLFW_EXAMPLE" withExamples)
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

    wrapProgram $bin/bin/gltfview${binExtension} \
      --set BRENDER_PATH $data/share/brender-samples/dat

    wrapProgram $bin/bin/brintro${binExtension} \
      --set BRENDER_PATH $data/share/brender-samples/brintro


    cp ${finalAttrs.src}/examples/tutorials/dat/* $data/share/brender-samples/tutorials

    for i in {1,2,3,4,5,6,7,8,9,10}; do
      wrapProgram $bin/bin/brtutor$i${binExtension} \
        --set BRENDER_PATH $data/share/brender-samples/tutorials
    done

    for i in minzb minzs sheet rdemo forest fogdem mirror spacerig; do
      if  [[ -e ${finalAttrs.src}/examples/samples/$i/dat ]]; then
        mkdir -p $data/share/brender-samples/samples/$i
        cp ${finalAttrs.src}/examples/samples/$i/dat/* $data/share/brender-samples/samples/$i
        wrapProgram $bin/bin/$i${binExtension} \
          --set BRENDER_PATH $data/share/brender-samples/samples/$i
      fi
    done
  '';

  doCheck = runTests;

  outputs = [ "bin" "lib" "dev" "out" ] ++ (lib.optionals withExamples [
    "data"
  ]);
})
