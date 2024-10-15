{ SDL2, fetchFromGitHub }:

SDL2.overrideAttrs(old: {
  pname = "SDL3";
  version = "3.1.3";

  src = fetchFromGitHub {
    owner = "libsdl-org";
    repo = "SDL";
    rev = "preview-3.1.3";
    hash = "sha256-S7yRcLHMPgq6+gec8l+ESxp2dJ+6Po/UNsBUXptQzMQ=";
  };
})