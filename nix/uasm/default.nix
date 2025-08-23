{ uasm }:
uasm.overrideAttrs(old: {
  makeFlags = old.makeFlags ++ [ "DJGPP=1" ];
  patches = (old.patches or []) ++ [
    ./0001-Makefile-Linux-GCC-OSX-Clang-64-allow-enabling-djgpp.patch
    ./0002-H-globals-add-SFORMAT_DJGPP-from-JWasm.patch
    ./0003-fixup-fix-djgpp-compile-error.patch
  ];
  meta = old.meta // { license = []; };
})
