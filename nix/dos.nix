{ stdenvNoCC
, lib
, version
, djgpp
, djgppPrefix
, systemProcessor
, cmake
, perl
, uasm
, wineserverHook
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
    uasm
    wineserverHook
  ];

  cmakeFlags = [
    (lib.cmakeFeature "CMAKE_SYSTEM_NAME" "DOS")
    (lib.cmakeFeature "CMAKE_SYSTEM_PROCESSOR" systemProcessor)
    (lib.cmakeFeature "CMAKE_C_COMPILER" "${djgppPrefix}gcc")
    (lib.cmakeFeature "CMAKE_CXX_COMPILER" "${djgppPrefix}g++")
    (lib.cmakeFeature "CMAKE_STRIP" "${djgppPrefix}strip")
    (lib.cmakeFeature "CMAKE_RANLIB" "${djgppPrefix}ranlib")
    (lib.cmakeFeature "CMAKE_AR" "${djgppPrefix}ar")
    (lib.cmakeFeature "CMAKE_ASM_MASM_COMPILER" "uasm")
    (lib.cmakeBool "DJGPP" true)

    (lib.cmakeBool "BRENDER_BUILD_SOFT" true)
    (lib.cmakeBool "BRENDER_BUILD_TOOLS" false)
    (lib.cmakeBool "BRENDER_BUILD_EXAMPLES" true)
    (lib.cmakeBool "BRENDER_DISABLE_INSTALL" true)
    (lib.cmakeBool "BRENDER_DISABLE_FINDSDL" true)
  ];
}
