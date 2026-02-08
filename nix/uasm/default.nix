{ pkgs }:
(pkgs.uasm.override { gcc13Stdenv = pkgs.stdenv; }).overrideAttrs(old: {
  makeFlags = old.makeFlags ++ [ "DJGPP=1" ];
  patches = (old.patches or []) ++ [
    ./0001-Makefile-Linux-GCC-OSX-Clang-64-allow-enabling-djgpp.patch
    ./0002-H-globals-add-SFORMAT_DJGPP-from-JWasm.patch
    ./0003-fixup-fix-djgpp-compile-error.patch
  ];
  meta = old.meta // { license = []; };

  # Get the damned thing compiling with GCC 15
  env.CFLAGS = "-std=c99 -Wno-incompatible-pointer-types -Wno-implicit-function-declaration -Wno-int-conversion";
})
