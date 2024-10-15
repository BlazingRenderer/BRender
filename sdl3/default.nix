{
  lib,
  stdenv,
  config,
  fetchFromGitHub,
  nix-update-script,
  pkg-config,
  cmake,
  libGLSupported ? lib.elem stdenv.hostPlatform.system mesa.meta.platforms,
  openglSupport ? libGLSupported,
  libGL,
  alsaSupport ? stdenv.hostPlatform.isLinux && !stdenv.hostPlatform.isAndroid,
  alsa-lib,
  x11Support ? !stdenv.hostPlatform.isWindows && !stdenv.hostPlatform.isAndroid,
  libX11,
  xorgproto,
  libICE,
  libXi,
  libXScrnSaver,
  libXcursor,
  libXinerama,
  libXext,
  libXxf86vm,
  libXrandr,
  waylandSupport ? stdenv.hostPlatform.isLinux && !stdenv.hostPlatform.isAndroid,
  wayland,
  wayland-protocols,
  wayland-scanner,
  drmSupport ? false,
  libdrm,
  mesa,
  libxkbcommon,
  dbusSupport ? stdenv.hostPlatform.isLinux && !stdenv.hostPlatform.isAndroid,
  dbus,
  udevSupport ? stdenv.hostPlatform.isLinux && !stdenv.hostPlatform.isAndroid,
  udev,
  ibusSupport ? false,
  ibus,
  libdecorSupport ? stdenv.hostPlatform.isLinux && !stdenv.hostPlatform.isAndroid,
  libdecor,
  pipewireSupport ? stdenv.hostPlatform.isLinux && !stdenv.hostPlatform.isAndroid,
  pipewire, # NOTE: must be built with SDL2 without pipewire support
  pulseaudioSupport ?
    config.pulseaudio or stdenv.hostPlatform.isLinux && !stdenv.hostPlatform.isAndroid,
  libpulseaudio,
  AudioUnit,
  Cocoa,
  CoreAudio,
  CoreServices,
  ForceFeedback,
  OpenGL,
  audiofile,
  libiconv,
  withStatic ? stdenv.hostPlatform.isMinGW,
  # passthru.tests
  testers,
  guile-sdl2,
  jazz2,
  SDL2_ttf,
  SDL2_net,
  SDL2_gfx,
  SDL2_sound,
  SDL2_mixer,
  SDL2_image,
  python3Packages,
}:

# NOTE: When editing this expression see if the same change applies to
# SDL expression too

stdenv.mkDerivation (finalAttrs: {
  pname = "SDL3";
  version = "3.1.3";

  src = fetchFromGitHub {
    owner = "libsdl-org";
    repo = "SDL";
    rev = "preview-${finalAttrs.version}";
    hash = "sha256-S7yRcLHMPgq6+gec8l+ESxp2dJ+6Po/UNsBUXptQzMQ=";
  };
  dontDisableStatic = if withStatic then 1 else 0;
  outputs = [
    "out"
    "dev"
  ];
  outputBin = "dev"; # sdl-config

  strictDeps = true;

  depsBuildBuild = [ pkg-config ];

  nativeBuildInputs =
    [ pkg-config cmake ]
    ++ lib.optionals waylandSupport [
      wayland
      wayland-scanner
    ];

  dlopenPropagatedBuildInputs =
    [ ]
    # Propagated for #include <GLES/gl.h> in SDL_opengles.h.
    ++ lib.optional (openglSupport && !stdenv.hostPlatform.isDarwin) libGL
    # Propagated for #include <X11/Xlib.h> and <X11/Xatom.h> in SDL_syswm.h.
    ++ lib.optionals x11Support [ libX11 ];

  propagatedBuildInputs =
    lib.optionals x11Support [ xorgproto ] ++ finalAttrs.dlopenPropagatedBuildInputs;

  dlopenBuildInputs =
    lib.optionals alsaSupport [
      alsa-lib
      audiofile
    ]
    ++ lib.optional dbusSupport dbus
    ++ lib.optional libdecorSupport libdecor
    ++ lib.optional pipewireSupport pipewire
    ++ lib.optional pulseaudioSupport libpulseaudio
    ++ lib.optional udevSupport udev
    ++ lib.optionals waylandSupport [
      wayland
      libxkbcommon
    ]
    ++ lib.optionals x11Support [
      libICE
      libXi
      libXScrnSaver
      libXcursor
      libXinerama
      libXext
      libXrandr
      libXxf86vm
    ]
    ++ lib.optionals drmSupport [
      libdrm
      mesa
    ];

  buildInputs =
    [ libiconv ]
    ++ finalAttrs.dlopenBuildInputs
    ++ lib.optional ibusSupport ibus
    ++ lib.optionals waylandSupport [ wayland-protocols ]
    ++ lib.optionals stdenv.hostPlatform.isDarwin [
      AudioUnit
      Cocoa
      CoreAudio
      CoreServices
      ForceFeedback
      OpenGL
    ];

  enableParallelBuilding = true;

  cmakeFlags = [
    (lib.cmakeBool "DISABLE_OSS" true)
    (lib.cmakeBool "SDL_X11" x11Support)
    (lib.cmakeBool "SDL_ALSA" alsaSupport)
    (lib.cmakeBool "SDL_OPENGLES" stdenv.hostPlatform.isWindows)
    (lib.cmakeBool "SDL_TEST_LIBRARY" stdenv.hostPlatform.isDarwin)
  ];

  # SDL is weird in that instead of just dynamically linking with
  # libraries when you `--enable-*` (or when `configure` finds) them
  # it `dlopen`s them at runtime. In principle, this means it can
  # ignore any missing optional dependencies like alsa, pulseaudio,
  # some x11 libs, wayland, etc if they are missing on the system
  # and/or work with wide array of versions of said libraries. In
  # nixpkgs, however, we don't need any of that. Moreover, since we
  # don't have a global ld-cache we have to stuff all the propagated
  # libraries into rpath by hand or else some applications that use
  # SDL API that requires said libraries will fail to start.
  #
  # You can grep SDL sources with `grep -rE 'SDL_(NAME|.*_SYM)'` to
  # list the symbols used in this way.
  postFixup =
    let
      rpath = lib.makeLibraryPath (
        finalAttrs.dlopenPropagatedBuildInputs ++ finalAttrs.dlopenBuildInputs
      );
    in
    lib.optionalString (stdenv.hostPlatform.extensions.sharedLibrary == ".so") ''
      for lib in $out/lib/*.so* ; do
        if ! [[ -L "$lib" ]]; then
          patchelf --set-rpath "$(patchelf --print-rpath $lib):${rpath}" "$lib"
        fi
      done
    '';

  setupHook = ./setup-hook.sh;

  passthru = {
    inherit openglSupport;
    updateScript = nix-update-script {
      extraArgs = [
        "--version-regex"
        "release-(.*)"
      ];
    };
    tests = {
      pkg-config = testers.hasPkgConfigModules { package = finalAttrs.finalPackage; };
      inherit
        guile-sdl2
        jazz2
        SDL2_ttf
        SDL2_net
        SDL2_gfx
        SDL2_sound
        SDL2_mixer
        SDL2_image
        ;
      inherit (python3Packages) pygame pygame-ce pygame-sdl2;
    };
  };

  meta = with lib; {
    description = "Cross-platform multimedia library";
    mainProgram = "sdl3-config";
    homepage = "http://www.libsdl.org/";
    changelog = "https://github.com/libsdl-org/SDL/releases/tag/release-${finalAttrs.version}";
    license = licenses.zlib;
    platforms = platforms.all;
    maintainers = lib.teams.sdl.members;
    pkgConfigModules = [ "sdl2" ];
  };
})
