addSDL3Path () {
  if [ -e "$1/include/SDL3" ]; then
    export SDL3_PATH="${SDL3_PATH-}${SDL3_PATH:+ }$1/include/SDL3"
  fi
}

addEnvHooks "$hostOffset" addSDL3Path
