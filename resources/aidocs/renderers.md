# Renderer Details

## softrend

### State Management
- **Global static cache** (`scache`): `drivers/softrend/setup.c:34`, declared in `rend.h:216`. Contains model→screen matrix, view→model inverse, eye position, light data, ambient colour. Used by ALL rendering functions. Must be properly invalidated when renderer state changes.
- **Timestamp-based copy-on-write**: `StateCopy` (`rndstate.c:235-354`) compares dest/src timestamps and only copies changed fields. Optimizes state push/pop by avoiding redundant memory copies.
- **Stack limit**: `MAX_STATE_STACK = 32` (state.h:19). Exceeding this returns `BRE_OVERFLOW`.
- **TouchModelToView** (`state.c:513-521`): central invalidation for scache. Called from `modelMul` and `modelPopPushMul`. Sets `scache.valid_v2m`, `valid_m2s`, `valid_per_model` to `BR_FALSE`.

### Lighting Dispatch

Softrend's per-vertex lighting is in `light24.c` (24-bit) and `light8.c` (8-bit), with shared macros in `lightmac.h`.

The system uses a **function pointer array** built at render-setup time. `GenerateSurfaceFunctions` (`setup.c:777-896`) inspects `rend.block->vertex_components` (a bitmask of which components the rasteriser needs) and the renderer state (`lighting? prelighting? opacity_source?`) to build an array of `surface_fn` pointers:

```c
typedef void SURFACE_CALL surface_fn(
    br_renderer *self, br_vector3 *p, br_vector2 *map,
    br_vector3 *n, br_colour colour, br_scalar *comp);
```

Each combination of component mask bits and state flags selects a different function:

| Mask bits                                        | State                                                | Function |
|--------------------------------------------------|------------------------------------------------------|----------|
| `CM_R\|CM_G\|CM_B`, lighting on, prelighting off | `SurfaceColourLit` (`light24.c:144`)                 |
| `CM_R\|CM_G\|CM_B`, lighting on, prelighting on  | `SurfaceColourLitPrelit` (`light24.c:243`)           |
| `CM_UR\|CM_UG\|CM_UB` (unlit RGB)                | `SurfaceColourUnlit` (`light24.c:126`)               |
| `CM_A`, `opacity_source == BRT_GEOMETRY`         | `SurfaceAlphaPrealpha` (`alpha.c:19`)                |
| `CM_A`, `opacity_source != BRT_GEOMETRY`         | `SurfaceAlpha` (`alpha.c:13`)                        |
| component not requested                          | `SurfaceColourZero` (`light24.c:116`, writes offset) |
| 8-bit `CM_I` (`CM_IR\|CM_IG\|CM_IB`)             | `SurfaceIndexLit` (`light8.c:40`)                    |

The function pointers go into `renderer->state.cache.vertex_fns[]`. Per-vertex, the geometry dispatch functions (`genrend.c`) iterate this array:

```c
for(i = 0; i < renderer->state.cache.nvertex_fns; i++)
    renderer->state.cache.vertex_fns[i](renderer, vp_p, vp_map, vp_n,
        source_colour, tvp->comp);
```

`source_colour` is either `scache.colour` (BRT_SURFACE) or `rend.vertex_colours[v]` (BRT_GEOMETRY), selected by which `Vertex_SurfaceComponents*` variant was chosen. Two-sided variants also negate normals for back-facing vertices.

The 8-bit path has its own set of indexed-colour functions in `light8.c` that output a palette index instead of RGB scalars.

### Geometry Functions (`genrend.c`)
Four variants control which colour is passed to surface functions:
- `Vertex_SurfaceComponentsGeom` — colour_source=GEOMETRY, one-sided
- `Vertex_SurfaceComponentsSurf` — colour_source=SURFACE, one-sided
- `Vertex_SurfaceComponentsTwoSidedGeom` — GEOMETRY, two-sided
- `Vertex_SurfaceComponentsTwoSidedSurf` — SURFACE, two-sided

Two-sided variants negate normals for back-facing vertices.

### Scache Validity Flags
| Flag              | Meaning                             | Set TRUE by                  | Set FALSE by                             |
|-------------------|-------------------------------------|------------------------------|------------------------------------------|
| `valid_v2m`       | view_to_model is current            | `StaticCacheUpdate_PerModel` | `TouchModelToView`, `StateCopy` (matrix) |
| `valid_m2s`       | model_to_screen is current          | `ModelToScreenUpdate`        | `TouchModelToView`, `StateCopy` (matrix) |
| `valid_per_model` | per-model lighting cache is current | `V1Model_Render`             | `TouchModelToView`, `StateCopy` (matrix) |
| `valid_per_scene` | per-scene lighting cache is current | `V1Model_Render`             | (scene changes)                          |

### Opacity Baking
`v1model.c:806-808`: surface colour and opacity are merged into `scache.colour`:
```c
scache.colour = renderer->state.surface.colour & 0xFFFFFF;
scache.colour |= BrScalarToInt(BR_CONST_MUL(renderer->state.surface.opacity, 256)) << 24;
```
Alpha is stored in the upper byte. `SurfaceAlpha` extracts it via `BR_ALPHA(colour)`.

### Divert/Draw Logic
`v1model.c:791`: geometry is diverted to z-sort heap when z-sort is enabled. `z_sort` triggers when hidden surface `type == BRT_BUCKET_SORT` and `divert == BRT_ALL`; `z_sort_blended` triggers on `divert == BRT_BLENDED`. The actual divert decision is `z_sort || (z_sort_blended && rend.block->flags & BR_PRIMF_BLENDED)`. Non-diverted geometry renders immediately.

---

## glrend

### State Stack
- `drivers/glrend/state.c`: pointer-based pool with `state.current`. `StateGLPush` copies entire current state to next slot. `StateGLPop` just decrements pointer.
- `MAX_STATE_STACK = 64` (state.h:6).
- No timestamp optimization — always full copy on push.
- No global cache — per-renderer-instance cache recomputed fresh.

### Lighting Pipeline
- **Vertex shader** (`brender.vert.glsl`): transforms vertices, computes per-vertex Gouraud lighting via `accumulateLights()`. Outputs `vertexLighting` as `layout(location=4) out mat3` varyings (unclamped), with `[0]=Ambient, [1]=Diffuse, [2]=Specular`.
- **Fragment shader** (`brender.frag.glsl`): reads `lightA = vertexLighting[0]`, `lightD = vertexLighting[1]`, `lightS = vertexLighting[2]`. Adds prelighting vertex colour when enabled, applies fullbright (`lightD += 1.0`) when no lighting, then computes `(lightA + lightD) * surfaceColour * texColour + lightS`. Adds fog, clamps to [0,1].
- **Colour key**: gated by `disable_colour_key` uniform (inverted from `PRIMF_COLOUR_KEY`). Discards when `texColour.rgb == vec3(0,0,0)`.
- **Phong path**: when `shading_mode == SHADING_MODE_PHONG`, `accumulateLights` runs again in fragment shader with per-fragment normals.

### Shader Preprocessing
Defined in `drivers/glrend/CMakeLists.txt`. Uses `#include` directives concatenated at build time, producing `.processed.glsl` files that get embedded as C string headers.

### Geometry Storage
- `gstored.c`: model data loaded into VBOs. `build_vbo` creates GPU vertex arrays. Vertex colour alpha hardcoded to 255.
- `BufferStoredGLAllocate`: allocates from ring buffer. `BufferStoredUpdate` uploads to GPU.
- UBO-based uniforms (`shader_main.h` declares the UBO structs; `buffer_ring.c` implements the ring buffer, managed via `renderer.h`).

### VBO Vertex Format
```c
typedef struct gl_vertex_f {
    br_vector3_f p;     // position
    br_vector2_f map;   // UV
    br_vector3_f n;     // normal
    uint8_t      c[4];  // colour RGBA (alpha = 255)
} gl_vertex_f;
```
Colour is `GL_UNSIGNED_BYTE, GL_TRUE` (normalized to [0,1] by GPU).

---

## glrend1x

### GL 1.2 Compatibility Constraints
- **No FB objects**: backbuffer from front screen, depth buffer from backbuffer.
- **No shaders, no VBO/VAO**: all geometry via `glBegin`/`glEnd` immediate mode.
- **No `EXT_bgra`/`EXT_packed_pixels`**: GL 1.2 has `GL_BGRA`/`GL_UNSIGNED_SHORT_*` in core.
- **`glColor4f` clamps to [0,1]** per the GL 1.2 spec. No way to pass unclamped per-vertex colours through fixed-function.
- **`glTexCoord` is unclamped** — the only unclamped per-vertex attribute.

### Texture Handling
- Always-texture with white 1×1 fallback (`pstate.c`). `GL_TEXTURE_2D` always enabled with `GL_MODULATE`.
- Indexed textures expanded to RGBA at upload time (`util.c`).
- `BRT_CLUT_O` palette override supported.
- Texture filtering set after bind in `renderBegin` (not before, to avoid operating on wrong texture).
- Mipmap variants dropped (no mipmap generation, incomplete textures render black).

### Blend / Transparency
- `PRIMF_DECAL` removed — all primitives use `GL_MODULATE`. glrend also ignores DECAL.
- Colour key alpha test disabled for blended primitives (`is_blended` → skip `GL_ALPHA_TEST`; alpha is blend factor, not transparency mask).
- Vertex alpha from `comp_f[C_A]` when `PRIMF_BLEND` and `C_A > 0.001f`, falls back to `1.0f` for opaque.
- Depth test always enabled when `out.depth != NULL`.
- `glDepthRange(1.0, 0.0)` removed — broke Croc. Ortho `(0,w,h,0,-1,1)` with default depth range handles BRender Z convention.

### Block Definitions
Primitive blocks generated via `MAKE_BLOCKS` macro in `pstate.c`. Four variants per primitive type:
- `block_triangle` / `block_triangle_blended` / `block_triangle_subdiv` / `block_triangle_blended_subdiv`
- `block_line` (same four variants)
- `block_point` (same four variants)
- Base block has no extra flags; `_blended` adds `BR_PRIMF_BLENDED`; `_subdiv` adds `BR_PRIMF_SUBDIVIDE`; `_blended_subdiv` adds both.

### Fog
`glFogCoordfEXT(v->comp_f[C_W])` per-vertex when `PRIMF_FOG` and `fog_type != BRT_NONE`. `(1 << C_W)` in convert_mask.

---

## pentprim

### Platform Constraints
- x86 assembly only — requires 32-bit x86-compatible build.
- Disabled by default; enable with `-DBRENDER_BUILD_SOFT=ON`.
- 8-bit indexed, 15-bit, 16-bit, and 24-bit true-colour rasterisers.

### Lighting Assumptions
- CM_A is included in component masks for 32 blocks that use `constant_alpha` in `mmx_t15.ifg`/`mmx_t16.ifg`, but the assembly rasterisers handle alpha via internal constants or lookup tables rather than calling `SurfaceAlpha`.
- Colour key: hardcoded `test bl,bl; jz skip` in all indexed textured rasterisers — palette index 0 always transparent, regardless of `BRT_COLOUR_KEY_B`.

### Blend Behaviour
- 8-bit blend uses a 2D lookup table, not arithmetic alpha from C_A.
- 24-bit `blendrgb` rasterisers use internal constants, not `SurfaceAlpha`.

---

## Pixelmap Cloning / Colour Keying

### Colour Key Convention
Palette index 0 is universally transparent across BRender:
- **pentprim**: hardcoded in assembly rasterisers
- **texconv** (`tools/texconv/`): `BR_ALPHA <= 127` → index 0
- **pmclone** (`core/pixelmap/pmclone.c`): index 0 reads as transparent, writes reserved for transparent output
- **glrend** shader: `if(texColour.rgb == vec3(0,0,0)) discard` post-CLUT, gated by `PRIMF_COLOUR_KEY`

### BrPixelmapConvert
Replaced the old `DevicePixelmapMemCloneTyped` / `BRT_CLONE` dispatch chain. Only handles memory pixelmaps (no driver backs `BRT_CLONE`). Uses `br_pixelmap_convert_options` struct with `index_alpha_threshold` (default 127), `enable_colour_key`, and `colour_key`. See `docs/pixelmap-colour-keying.md` for full documentation.

### Quantizer API
`BrQuantMakePalette(base, num_entries, palette)` with Wu's algorithm. `base` parameter reserves palette slots. `BrQuantPrepareMapping` builds inverse LUT. `BrQuantMapColours` maps RGB→index. Used in `pmclone.c` with `base=1` to reserve slot 0 for transparent.
