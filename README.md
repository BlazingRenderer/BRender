# Blazing Renderer (BRender) 1.4.0

This is an updated, modernized fork of [BRender](https://blazingrender.net/),
based on the following source code releases:

- [BRender-1997](https://github.com/foone/BRender-1997)
- [BRender-v1.3.2](https://github.com/foone/BRender-v1.3.2)

There are OpenGL and SDL2/SDL3 drivers, as well as an x86-specific software rendering
backend used as the reference implementation.

Many sample applications are included, as well as most of the original utilities
released with the engine.

Enjoy!

## Building

Building is done through CMake.

```
mkdir cmake-build && cd cmake-build
cmake ..
make
```

### macOS (Apple Silicon)

Install dependencies via [Homebrew](https://brew.sh):

```sh
brew install cmake sdl3 glslang spirv-cross
```

Then build with SDL3:

```sh
mkdir cmake-build && cd cmake-build
cmake .. -DBRENDER_USE_SDL3=ON
make -j$(sysctl -n hw.logicalcpu)
```

The x86 software renderer (`pentprim`) cannot be built on ARM — the OpenGL renderer is used instead. Everything else builds and runs correctly, including the sample apps.

To try it out, run the forest demo from the build directory:

```sh
cd examples/samples/forest
./forest
```

## License

This is released under the MIT license.

Copyright (c) 1992-1998 Argonaut Technologies Limited

Copyright (c) 2014-2025 Zane van Iperen

Copyright (c) 2022-2025 erysdren (it/its)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
