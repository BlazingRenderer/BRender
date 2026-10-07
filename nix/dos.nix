{ stdenvNoCC
, lib
, version
, djgpp
, djgppPrefix
, systemProcessor
, cmake
, perl
, clang-tools
}:
stdenvNoCC.mkDerivation {
  inherit version;

  pname = "brender-dos";

  src = ../.;

  passthru.devTools = [
    clang-tools
  ];

  nativeBuildInputs = [
    djgpp
    cmake
    perl
  ];

  cmakeFlags = [
    (lib.cmakeFeature "CMAKE_SYSTEM_NAME" "DOS")
    (lib.cmakeFeature "CMAKE_SYSTEM_PROCESSOR" systemProcessor)
    (lib.cmakeFeature "CMAKE_C_COMPILER" "${djgppPrefix}gcc")
    (lib.cmakeFeature "CMAKE_CXX_COMPILER" "${djgppPrefix}g++")
    (lib.cmakeFeature "CMAKE_STRIP" "${djgppPrefix}strip")
    (lib.cmakeFeature "CMAKE_RANLIB" "${djgppPrefix}ranlib")
    (lib.cmakeFeature "CMAKE_AR" "${djgppPrefix}ar")
    (lib.cmakeBool "DJGPP" true)

    (lib.cmakeBool "BRENDER_BUILD_TOOLS" false)
    (lib.cmakeBool "BRENDER_BUILD_EXAMPLES" true)
    (lib.cmakeBool "BRENDER_DISABLE_INSTALL" true)
    (lib.cmakeBool "BRENDER_DISABLE_FINDSDL" true)
  ];

  installPhase = ''
    runHook preInstall

    for i in checkerboard8.mat checkerboard8.pix cube.dat shade.tab std.pal doscube.exe; do
      install -Dm644 "examples/doscube/$i" "$out/$i"
    done

    runHook postInstall
  '';
}
