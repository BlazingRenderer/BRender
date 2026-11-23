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
, vulkan-loader
, vulkan-validation-layers
, withTools ? true
, withExamples ? false
, useGlRuntime ? null
}: let
  binExtension = stdenv.hostPlatform.extensions.executable;

  runTests = withTools && stdenv.buildPlatform.canExecute stdenv.hostPlatform;

  # The corpus and the witnesses create a GL or Vulkan device. On Linux with no GPU
  # that needs mesa's llvmpipe and lavapipe named, because the loader looks for them
  # under /run/opengl-driver, which neither a build sandbox nor a CI runner provides;
  # darwin takes GL from the system. So the environment follows the checks rather
  # than being a second opt-in beside them: `useGlRuntime` overrides it, and null
  # means "wherever the checks run", which is the case that has to work.
  glRuntime = if useGlRuntime != null then useGlRuntime else runTests && stdenv.hostPlatform.isLinux;

  glRuntimeEnv = lib.optionalString glRuntime ''
    export LIBGL_ALWAYS_SOFTWARE=1
    export GALLIUM_DRIVER=llvmpipe
    export LIBGL_DRIVERS_PATH=${mesa}/lib/dri
    export __EGL_VENDOR_LIBRARY_FILENAMES=${mesa}/share/glvnd/egl_vendor.d/50_mesa.json
    export LD_LIBRARY_PATH=${libglvnd}/lib:${mesa}/lib''${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}

    export VK_DRIVER_FILES=$(echo ${mesa}/share/vulkan/icd.d/lvp_icd.*.json)
    export VK_LAYER_PATH=${vulkan-validation-layers}/share/vulkan/explicit_layer.d
    export LD_LIBRARY_PATH=${vulkan-loader}/lib:${vulkan-validation-layers}/lib:$LD_LIBRARY_PATH
  '';
in
assert glRuntime -> runTests && stdenv.hostPlatform.isLinux;
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

  passthru.vulkan-loader = vulkan-loader;

  passthru.glRuntimeEnv = glRuntimeEnv;

  buildInputs = [
    sdl3
  ] ++ lib.optionals withExamples [
    glfw
  ];

  checkInputs = lib.optionals runTests [
    mesa
    libglvnd
    vulkan-loader
    vulkan-validation-layers
  ];

  preCheck = glRuntimeEnv;

  doCheck = runTests;

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

  outputs = [ "bin" "lib" "dev" "out" ] ++ (lib.optionals withExamples [
    "data"
  ]);

  # The checks are not the devshell: only the rendertest shell asks for this on
  # entry, so it follows `useGlRuntime` and not `glRuntime`.
  shellHook = lib.optionalString (useGlRuntime == true) glRuntimeEnv;
})
