# Rendering Architecture

## Driver Relationships

```
              ┌─────────────────────┐
              │   core/v1db/        │  Scene traversal, actor hierarchy,
              │   render.c          │  BrOnScreenCheck, render style dispatch
              └──────────┬──────────┘
                         │  RendererStatePush/Pop, ModelMul, BrDbModelRender
              ┌──────────┴──────────┐
              ▼                     ▼
   ┌─────────────────────┐  ┌─────────────────────┐
   │  drivers/softrend/  │  │  drivers/glrend/    │  Independent GPU driver —
   │  v1model.c          │  │  brender.vert.glsl  │  own T&L (shaders),
   │  genrend.c          │  │  brender.frag.glsl  │  own state stack, VBOs
   │  light24.c          │  │  v1model.c          │
   │  setup.c            │  │  cache.c            │
   │  rndstate.c         │  │  gstored.c          │
   │                     │  └─────────────────────┘
   │  CPU T&L — outputs  │
   │  brp_vertex buffer  │
   └──────────┬──────────┘
              │  Per-vertex output via brp_vertex (comp[NUM_COMPONENTS])
              ▼
   ┌──────────┼──────────┐
   ▼                     ▼
   ┌──────────┐          ┌──────────┐
   │ pentprim │          │ glrend1x │  Rasterisers — consume computed
   │ (x86 asm)│          │ (GL 1.2  │  vertices, emit pixels
   │8/15/16/24│          │ imm.mode)│
   └──────────┘          └──────────┘
```

- **softrend** does all CPU-side work: transforms, culling, clipping, per-vertex lighting, depth biasing. It builds a vertex buffer (`brp_vertex` array in `rend.temp_vertices`) and hands it to the rasteriser via `brp_block` dispatch.
- **glrend** is a fully independent driver — it does its own T&L on the GPU (vertex/fragment shaders), maintains its own state stack, and does not use softrend at all.
- **glrend1x** is a hybrid: softrend does T&L, glrend1x rasterises via GL 1.2 immediate mode (`glBegin`/`glEnd`, `glColor4f`, `glTexCoord2f`).
- **pentprim** is the traditional x86 assembly rasteriser for 8/15/16/24-bit colour modes.

## Render Pipeline (softrend + rasteriser)

1. **Scene traversal** (`core/v1db/render.c:actorRender`): walks actor hierarchy depth-first
   - For non-identity actors: `RendererStatePush(BR_STATE_MATRIX)` → `RendererModelMul(actor_transform)` → render model → recurse children → `RendererStatePop`
   - For identity actors: skip push/pop, use current state
   - Calls `BrDbModelRender` → `V1Model_Render`

2. **Per-model setup** (`drivers/softrend/v1model.c:V1Model_Render`):
   - Updates global `scache` (static cache) from renderer state
   - Sets up `rend.vertex_colours`, `rend.face_colours` from model data
   - Calls `VertexGeometryFns()` to build vertex processing pipeline
   - Dispatches to geometry functions (`genrend.c`) for vertex processing

3. **Per-vertex processing** (`drivers/softrend/genrend.c`):
   - `Vertex_SurfaceComponentsGeom` (colour_source = BRT_GEOMETRY): passes `rend.vertex_colours[v]` as source colour
   - `Vertex_SurfaceComponentsSurf` (colour_source = BRT_SURFACE): passes `scache.colour` (material surface colour) for all vertices
   - Surface functions (e.g., `SurfaceColourLit`) compute per-vertex lighting into `comp[]`

4. **Rasterisation**:
   - The vertex buffer (`rend.temp_vertices`) is dispatched to rasteriser blocks via `brp_block` functions
   - For glrend1x: `TriangleRenderGL1x` → `EmitVertex` → `glColor4fv`/`glTexCoord2fv`/`glVertex3f`

## Key Types

### `brp_vertex` (`drivers/softrend/ddi/priminfo.h`)
```c
typedef union brp_vertex {
    br_int_32   flags;
    br_scalar   comp[NUM_COMPONENTS];  // Fixed/scalar output (lighting writes here)
    br_float    comp_f[NUM_COMPONENTS];// Float view (rasteriser reads here)
    br_fixed_ls comp_x[NUM_COMPONENTS];// Fixed-point view
    br_int_32   comp_i[NUM_COMPONENTS];// Integer view
} brp_vertex;
```
Since `br_scalar == br_float` (see `scalar.h`), `comp[]` and `comp_f[]` share the same memory — no conversion needed.

### Component Indices
- `C_X=1, C_Y=2, C_Z=3, C_W=4` — position (homogeneous W)
- `C_SX=5, C_SY=6, C_SZ=7` — screen position
- `C_U=8, C_V=9` — texture coordinates
- `C_I=10, C_A=10` — palette index / alpha (share slot)
- `C_R=11, C_G=12, C_B=13` — RGB colour
- `C_UI=10, C_UR=11, C_UG=12, C_UB=13` — unlit colour variants (share slots)
- `C_S=14, C_SW=14` — fog/specular coordinate
- `C_Q=15` — reciprocal of C_W (1/W)

## z-sort / Divert Phase

When `rend.block->flags & BR_PRIMF_BLENDED` is set **and** z-sort blending (BRT_BLENDED divert mode) is active, softrend diverts blended geometry to a z-sort heap instead of rendering inline. When BRT_ALL divert mode is active, all geometry is diverted regardless of blend flag. The heap is flushed at render end (`gv1buckt.c`), drawing blended primitives in back-to-front Z order after all opaque geometry.

The divert phase calls `renderBegin(no_render=true)` which skips GL state setup. In glrend1x, `glBegin` is also skipped when `no_render` is true. In pentprim, the begin still runs but suppresses state pushes. The actual bucket flush later calls `renderBegin(no_render=false)` with proper state.

## State Management (softrend)

### Global Static Cache (`scache`)
Defined in `drivers/softrend/setup.c:34`, declared in `rend.h:216`. A single global `_static_cache` used by all rendering operations. Contains:
- `model_to_screen` — combined model→screen matrix
- `view_to_model` — inverse model→view
- `eye_m` — eye position in model space
- Per-model lighting data, ambient colour, etc.

**Invalidation**: scache matrix-dependent flags (`valid_v2m`, `valid_m2s`, `valid_per_model`) are set to `BR_FALSE` by `TouchModelToView` (called from `modelMul`) and by `StateCopy` whenever the matrix state is copied, ensuring the cache is recomputed after any state change.

### State Stack
Softrend uses a timestamp-based copy-on-write approach. `StateCopy` compares dest/src timestamps and only copies fields that changed. The stack is limited to `MAX_STATE_STACK = 32` entries.

### State Masks
| Mask                   | Covers                                                                   |
|------------------------|--------------------------------------------------------------------------|
| `MASK_STATE_MATRIX`    | model_to_view, view_to_screen, view_to_environment                       |
| `MASK_STATE_SURFACE`   | ka, kd, ks, colour_source, lighting, prelighting, opacity, map_transform |
| `MASK_STATE_CULL`      | culling mode                                                             |
| `MASK_STATE_PRIMITIVE` | primitive state (texture, blend, etc.)                                   |
| `MASK_STATE_CACHE`     | cached surface/cull data                                                 |

## glrend (Independent Driver)

glrend does not use softrend. It has:
- **Own state stack** (`drivers/glrend/state.c`): pointer-based pool, not timestamp-based. `StateGLPush` copies the entire current state. `StateGLPop` just decrements the pointer.
- **Own T&L**: GPU vertex/fragment shaders. Per-vertex lighting via Gouraud (vertex shader), with optional Phong (per-fragment) path.
- **VBO-based geometry** (`gstored.c`): model data uploaded to GPU buffers, not CPU vertex arrays.
- **UBO-based uniforms** (`shader_main.h`): per-model and per-scene data passed via uniform buffer objects.
- **No global static cache**: per-renderer-instance caches recomputed fresh each frame.

## glrend1x (GL 1.2 Rasteriser)

### GL 1.2 Constraints
- Immediate mode only (`glBegin`/`glEnd`). No VBO, no VAO, no shaders.
- **`glColor4f` clamps to [0,1]** — a GL 1.2 spec requirement. Values > 1.0 are truncated before interpolation.
- **`glTexCoord` is unclamped** — the only per-vertex attribute that preserves arbitrary float values.
- **`glSecondaryColor3f` also clamps to [0,1]** — not useful as a workaround.
- Texture environment: `GL_MODULATE` only. `GL_COMBINE`/`GL_ADD` require extensions (ARB_texture_env_combine core in 1.3).

### Key Files
- `pstate.c`: `renderBegin`, `pushGLState`, block definitions, blend modes, texture filtering, part template entries
- `rendfunc.c`: `EmitVertex`, `TriangleRenderGL1x`, `LineRenderGL1x`, `PointRenderGL1x`
- `util.c`: `DeviceGL1xPixelmapToExistingGLTextureConvert` — texture upload, indexed expansion
- `sbuffer.c`: `BufferStoredGL1xAllocate` — lazy texture upload, `gl_tex` management

### Surface Colour Flow
- `colour_source == BRT_SURFACE`: softrend passes `scache.colour` (material colour) as the vertex colour to ALL vertices
- `colour_source == BRT_GEOMETRY`: softrend passes model vertex colours — typically white (255,255,255) for textured groups and material colour for untextured groups
- glrend1x reads `comp_f[C_R/G/B]` and passes to `glColor4f`, which is then modulated by the texture (GL_MODULATE)
