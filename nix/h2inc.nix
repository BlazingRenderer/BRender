{ buildGoLatestModule, fetchFromGitHub }:
buildGoLatestModule(finalAttrs: {
  pname = "h2inc";
  version = "1.0.0";

  src = fetchFromGitHub {
    owner = "BlazingRenderer";
    repo = "h2inc";
    tag = "v${finalAttrs.version}";
    hash = "sha256-N3Z5s4XPjH95JAzgpJHQYxIR5PZDCGmgE0KE4Bh9uMQ=";
  };

  vendorHash = null;

  meta = {
    mainProgram = "h2inc";
  };
})
