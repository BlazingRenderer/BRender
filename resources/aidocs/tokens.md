# Token/Template System

## Overview

BRender uses a token/value system for reading and writing typed properties on objects without knowing their struct layouts. The two main APIs are:

- **`ObjectQuery(obj, &value, BRT_SOME_TOKEN)`** — read a property from any `br_object`
- **`RendererPartSet(renderer, BRT_SURFACE, 0, BRT_AMBIENT_X, value)`** — write a property into a specific part of renderer state

Both use the same underlying mechanism: tokens (named enum constants with associated types) and templates (lookup tables mapping tokens to struct field offsets with conversion rules).

## Tokens

### Definition

Tokens are defined in `core/inc/pretok.tok` using a custom DSL with 47 type declarations:

```
$boolean                _B   b
$float                  _F   f
$token                  _T   t
$colour_rgb             _RGB rgb
$int_8                  _I8  i8
$uint_8                 _U8  u8
$int_16                 _I16 i16
$uint_16                _U16 u16
$int_32                 _I32 i32
$uint_32                _U32 u32
$int_64                 _I64 i64
$uint_64                _U64 u64
$fixed                  _X   x
$angle                  _A   a
$object                 _O   o
$string                 _STR str
$constant_string        _CSTR cstr
...plus vector/matrix/pointer/list variants
```

The first column is the type name; the second is the suffix appended to token names; the third is the `br_value` union member. Example output:

```
AMBIENT       fixed               → BRT_AMBIENT_X (type: BRT_FIXED)
AMBIENT       float               → BRT_AMBIENT_F (type: BRT_FLOAT)
MODEL_TO_VIEW matrix34_float      → BRT_MODEL_TO_VIEW_M34_F
```

Type suffixes are appended to base names. The `tokgen.pl` Perl script processes this into:
- `pretok.h` — the `br_token` enum with all `BRT_*` constants
- `pretok.c` — a static `br_token_entry[]` array with identifier strings, types, and base lengths
- `toktype.c` — a type table (suffix → type enum → size in `br_value` union)

### Runtime Token Creation

`BrTokenCreate(identifier, type)` (`core/fw/token.c:100-168`) — registers a new token at runtime, assigns `fw.next_free_token++`. If `type == BR_NULL_TOKEN`, the type is inferred from the suffix. Used by drivers to register device-specific tokens (e.g., `OPENGL_TEXTURE_U32`).

`BrTokenFind(pattern)` (`core/fw/token.c:224-236`) — looks up a token by glob pattern.

### The `br_value` Union

Every token's type corresponds to a member of `br_value` (`core/inc/token.h`):

```c
typedef union br_value_tag {
    void                  *p;
    br_intptr_t            pi;
    br_uintptr_t           pu;
    br_boolean             b;
    br_token               t;
    br_int_8               i8;
    br_uint_8              u8;
    br_int_16              i16;
    br_uint_16             u16;
    br_int_32              i32;
    br_uint_32             u32;
    br_int_64              i64;
    br_uint_64             u64;
    br_fixed_ls            x;
    br_float               f;
    br_scalar              s;
    br_angle               a;
    br_colour              rgb;
    struct br_object      *o;
    void                  *h;
    br_vector2            *v2;
    br_vector3            *v3;
    br_vector4            *v4;
    br_matrix23           *m23;
    br_matrix34           *m34;
    br_matrix4            *m4;
    /* ...plus integer/fixed/float variants of vectors and matrices,
     *      string types, and list types (object/token/pointer/token_value) */
    const char            *cstr;
} br_value;
```

## Templates

### Template Entries

`br_tv_template_entry` (`core/fw/ddi/tokenmap.h:14-23`):

```c
typedef struct br_tv_template_entry {
    br_token     token;      // Token constant (e.g., BRT_AMBIENT_X), or 0 if resolved by name
    const char  *name;       // Token name string (for DEV() entries resolved at runtime)
    br_uintptr_t offset;     // Byte offset into state struct, or absolute value if BRTV_ABS
    br_int_16    flags;      // BRTV_QUERY | BRTV_SET | BRTV_ALL | BRTV_ABS
    br_int_16    conv;       // Conversion type (BRTV_CONV_*)
    br_uintptr_t conv_arg;   // Extra argument (bitmask, custom fn ptr, literal value)
    br_uintptr_t mask;       // Action mask accumulated on SET (for cache invalidation)
} br_tv_template_entry;
```

### Flags

| Flag                  | Meaning                                             |
|-----------------------|-----------------------------------------------------|
| `BRTV_QUERY` (0x0001) | Valid for query operations                          |
| `BRTV_SET` (0x0002)   | Valid for set operations                            |
| `BRTV_ALL` (0x0004)   | Valid for query-all                                 |
| `BRTV_ABS` (0x0008)   | `offset` is an absolute pointer, not a field offset |

### Common Conversion Types (subset of the 35-entry `BRTV_CONV_*` enum)

| Conv                    | Purpose                              | conv_arg        |
|-------------------------|--------------------------------------|-----------------|
| `BRTV_CONV_COPY`        | Direct memcpy (size from token type) | (unused)        |
| `BRTV_CONV_DIRECT`      | Return `conv_arg` as the value       | literal value   |
| `BRTV_CONV_BIT`         | Set/clear a bit in a `uint32` field  | bitmask         |
| `BRTV_CONV_NOT_BIT`     | Inverted bit                         | bitmask         |
| `BRTV_CONV_FIXED_FLOAT` | Fixed-point ↔ float                  | (unused)        |
| `BRTV_CONV_FLOAT_FIXED` | Float ↔ fixed-point                  | (unused)        |
| `BRTV_CONV_CUSTOM`      | Call a custom callback               | `br_tv_custom*` |
| `BRTV_CONV_BOOL_TOKEN`  | Bool ↔ specific token                | token constant  |
| `BRTV_CONV_STRING`      | String copy                          | (unused)        |
| `BRTV_CONV_LIST`        | Pointer list of tokens               | (unused)        |
| `BRTV_CONV_PTR`         | Raw pointer                          | (unused)        |
| `BRTV_CONV_M34_COPY`    | Copy `br_matrix34`                   | (unused)        |
| `BRTV_CONV_M4_COPY`     | Copy `br_matrix4`                    | (unused)        |

Additional types not shown above include `BRTV_CONV_NOP`, `BRTV_CONV_V[2-4]_COPY`, `BRTV_CONV_M23_COPY`, `BRTV_CONV_I32_I8`/`_I16`/`_U8`/`_U16`, and all fixed/float conversion variants for vectors and matrices.

### Template Allocation and Lookup

`BrTVTemplateAllocate(res, entries, n_entries)` (`core/fw/tokenval.c:16`) wraps a static entry array into a heap-allocated `br_tv_template`. The entry array can be `static const` — only the wrapper needs allocation.

On first query/set, `templateMakeMap()` (`core/fw/tokenval.c:53-128`) builds two O(1) lookup arrays:

```c
// Token value → template entry index:
int o = token_value - template->map_base;
entry = template->map_set_entry[o];   // for SET
entry = template->map_query_entry[o]; // for QUERY
```

The range `[map_base, map_base + n_map_entries)` covers the min/max tokens in the entry array. A sparse token space wastes memory, but most token ranges are dense.

### BRT() / DEV() Macros

Each driver defines helper macros in its `drv.h`:

```c
#define BRT(t) BRT_##t, 0       // Built-in token: {token = BRT_DDI_VERSION_U32, name = NULL}
#define DEV(t) 0, #t            // Device-specific: {token = 0, name = "OPENGL_TEXTURE_U32"}
```

`BRT(t)` fills in the compiled token constant. `DEV(t)` leaves token as 0 and sets the name string — it's resolved at runtime via `BrTokenFind()` during `templateResolveNames()`.

## br_object Query Pipeline

Every `br_object` has a `templateQuery()` virtual method that returns its template. The public API calls through the dispatch table:

```
ObjectQuery(device, &value, BRT_OPENGL_VERSION_MAJOR_U8)
  │
  ▼  [dev_objs.c — classgen-generated convenience wrapper]
  BR_CMETHOD_CALL(br_object, query, device)(device, &value, token)
  │
  ▼  [device->dispatch->_query]
  br_object_query(self, &value, token)            // core/fw/object.c:15
  │
  ├── ObjectTemplateQuery(self)
  │     │
  │     ▼  [self->dispatch->_templateQuery]
  │     device->templates.deviceTemplate (lazy-allocated on first call)
  │
  └── BrTokenValueQuery(&value, NULL, 0, token, self, template)
        │
        ├── templateMakeMap(template) if not yet cached
        ├── o = token - template->map_base         // array index
        ├── entry = template->map_query_entry[o]
        └── ValueQuery(&value, self, entry)        // apply conversion
```

### Device Template Entry Examples

**glrend** (`drivers/glrend/device.c:28-44`) — exposes DDI version, OpenGL requirements, and metadata:

```c
static br_tv_template_entry deviceTemplateEntries[] = {
    {BRT(IDENTIFIER_CSTR),         F(identifier),    Q | A, BRTV_CONV_COPY,   0                      },
    {BRT(VERSION_U32),             0,                Q | A, BRTV_CONV_DIRECT, DEVICE_VERSION         },
    {BRT(BRENDER_VERSION_U32),     0,                Q | A, BRTV_CONV_DIRECT, __BRENDER__            },
    {BRT(DDI_VERSION_U32),         0,                Q | A, BRTV_CONV_DIRECT, __BRENDER_DDI__        },
    {BRT(CREATOR_CSTR),            A(deviceCreator), Q | A, BRTV_CONV_COPY,   0                      },
    {BRT(TITLE_CSTR),              A(deviceTitle),   Q | A, BRTV_CONV_COPY,   0                      },
    {BRT(PRODUCT_CSTR),            A(deviceProduct), Q | A, BRTV_CONV_COPY,   0                      },
    {BRT(OPENGL_VERSION_MAJOR_U8), 0,                Q | A, BRTV_CONV_DIRECT, 3                      },
    {BRT(OPENGL_VERSION_MINOR_U8), 0,                Q | A, BRTV_CONV_DIRECT, 3                      },
    {BRT(OPENGL_PROFILE_T),        0,                Q | A, BRTV_CONV_DIRECT, BRT_OPENGL_PROFILE_CORE},
};
```

When `offset == 0` and `BRTV_ABS` is not set, the value comes directly from `conv_arg`. This is the pattern for version constants.

### Cross-Device Token Query

**sdl3dev** queries the glrend device for OpenGL requirements before creating a window (`drivers/sdl3dev/glrend.c:146-184`):

```c
br_token_value gltv[] = {
    {.t = BRT_OPENGL_PROFILE_T,        .v = 0},
    {.t = BRT_OPENGL_VERSION_MAJOR_U8, .v = 0},
    {.t = BRT_OPENGL_VERSION_MINOR_U8, .v = 0},
    {.t = BR_NULL_TOKEN,               .v = 0},
};
ObjectQueryMany(gldev, gltv, NULL, 0, &count);
// Use gltv[0].v.t (profile), gltv[1].v.u8 (major), gltv[2].v.u8 (minor)
```

This is how the platform layer discovers what the rendering driver needs without hardcoding version assumptions.

### Custom Query Callbacks

When a value needs runtime computation (not a simple field read), use `BRTV_CONV_CUSTOM`:

```c
static br_error custom_query(br_value *pvalue, void **extra, br_size_t *pextra_size,
                             void *block, const br_tv_template_entry *tep)
{
    const br_device_pixelmap *self = block;
    if(tep->token == BRT_OPENGL_TEXTURE_U32) {
        pvalue->u32 = self->asBack.glTex;  // read the GL texture handle
        return BRE_OK;
    }
    return BRE_UNKNOWN;
}

// Template entry:
{DEV(OPENGL_TEXTURE_U32), 0, Q | A, BRTV_CONV_CUSTOM, (br_uintptr_t)&custom_query_struct}
```

## Renderer Part Templates

Renderer state is partitioned into named parts (`BRT_SURFACE`, `BRT_MATRIX`, `BRT_LIGHT`, etc.). Each part has its own `br_tv_template` with entries mapping sub-tokens to struct field offsets.

### Part Set Pipeline

```
RendererPartSet(renderer, BRT_SURFACE, 0, BRT_AMBIENT_X, {.x = BrFloatToFixed(0.5)})
  │
  ▼  [dev_objs.c — convenience wrapper]
  BR_CMETHOD_CALL(br_renderer, partSet, renderer)(renderer, BRT_SURFACE, 0, BRT_AMBIENT_X, value)
  │
  ▼  [renderer->dispatch->_partSet — e.g., br_renderer_soft_partSet (rndstate.c:51)]
  FindStateTemplate(self, &sp, BRT_SURFACE, 0)
    │
    └── Searches part_info[] for BRT_SURFACE
        Returns sp->partSurfaceTemplate (lazy-allocated)
  │
  ├── BrTokenValueSet(sp, &m, BRT_AMBIENT_X, value, template)
  │     │
  │     ├── o = BRT_AMBIENT_X - template->map_base
  │     ├── entry = template->map_set_entry[o]
  │     │   {token=BRT_AMBIENT_X, offset=offsetof(state_all, surface.ka),
  │     │    conv=BRTV_CONV_FIXED_SCALAR, mask=TM_PART}
  │     ├── ValueSet: memcpy to (char*)sp + offset
  │     └── *pcombined_mask |= TM_PART
  │
  └── TemplateActions(&self->state, BRT_SURFACE, 0, TM_PART)
        ├── state->surface.timestamp = Timestamp()
        └── (cache invalidation based on mask bits)
```

### Softrend Part Templates

Softrend defines its part templates in `drivers/softrend/state.c`:

```c
static const partInfo part_info[] = {
    P(CULL,           partCullTemplate),
    P(SURFACE,        partSurfaceTemplate),
    P(MATRIX,         partMatrixTemplate),
    P(ENABLE,         partEnableTemplate),
    P(HIDDEN_SURFACE, partHiddenSurfaceTemplate),
    P(BOUNDS,         partBoundsTemplate),
    P(LIGHT,          partLightTemplate),
    P(CLIP,           partClipTemplate),
};
```

Each entry maps a token (e.g., `BRT(AMBIENT_X)`) to a field offset (e.g., `F(surface.ka)`) with a conversion type. The same field can map to multiple tokens with different conversions:

```c
{BRT(AMBIENT_X), F(surface.ka), Q | S | AX, BRTV_CONV_FIXED_SCALAR, 0, TM_PART},
{BRT(AMBIENT_F), F(surface.ka), Q | S | AF, BRTV_CONV_FLOAT_SCALAR, 0, TM_PART},
```

Indexed parts (`BRT_LIGHT`, `BRT_CLIP`) adjust the state pointer by `index * sizeof(element)` so the offset is relative to the correct light/clip slot.

### Mask and Invalidation

The `mask` field accumulates across all fields modified in a `BrTokenValueSetMany` call. After all values are written, `TemplateActions` (softrend `state.c:429-508`) processes the combined mask:

| Mask bit                      | Action                               |
|-------------------------------|--------------------------------------|
| `TM_PART` (0x00001)           | Update per-part timestamp            |
| `TM_INDEX` (0x00002)          | Update timestamp for indexed part    |
| `TM_V2S_HINT` (0x00004)       | Bump `timestamp_v2s_hint`            |
| `TM_M2V` (0x00008)            | Bump `timestamp_m2v`                 |
| `TM_V2S` (0x00010)            | Bump `timestamp_v2s`                 |
| `TM_V2E` (0x00020)            | Bump `timestamp_v2e`                 |
| `TM_COPY_CACHE` (0x00040)     | Bump `timestamp_copy_cache`          |
| `TM_CACHE` (0x00080)          | Bump `timestamp_cache`               |
| `TM_CLEAR_M2V_HINT` (0x00100) | Set `model_to_view_hint = BRT_NONE`  |
| `TM_CLEAR_V2S_HINT` (0x00200) | Set `view_to_screen_hint = BRT_NONE` |
| `TM_INVALID_PS` (0x01000)     | `scache.valid_per_scene = BR_FALSE`  |
| `TM_INVALID_PM` (0x02000)     | `scache.valid_per_model = BR_FALSE`  |
| `TM_INVALID_V2M` (0x04000)    | `scache.valid_v2m = BR_FALSE`        |
| `TM_INVALID_M2S` (0x08000)    | `scache.valid_m2s = BR_FALSE`        |
| `TM_INVALID_CC` (0x10000)     | `state->cache.valid = BR_FALSE`      |

### glrend1x Part Templates

glrend1x defines its primitive part templates in `drivers/glrend1x/pstate.c:47-82`:

```c
static br_tv_template_entry partPrimitiveTemplateEntries[] = {
    {BRT(DEPTH_WRITE_B),  F(prim.flags),       Q | S | A, BRTV_CONV_BIT, PRIMF_DEPTH_WRITE, 0},
    {BRT(BLEND_B),        F(prim.flags),       Q | S | A, BRTV_CONV_BIT, PRIMF_BLEND,       1},
    {BRT(COLOUR_T),       F(prim.colour_type), Q | S | A, BRTV_CONV_COPY,                     0},
    {BRT(FOG_MIN_X),      F(prim.fog_min),     Q | S | A, BRTV_CONV_FIXED_SCALAR,             0},
    // ...
};

static br_tv_template_entry partOutputTemplateEntries[] = {
    {BRT(COLOUR_BUFFER_O), F(out.colour), Q | S | A, BRTV_CONV_COPY, 0},
    {BRT(DEPTH_BUFFER_O),  F(out.depth),  Q | S | A, BRTV_CONV_COPY, 0},
};
```

Templates are lazily allocated in `findTemplate()` (`pstate.c:400-420`) and cached on `device->templates`.

## Dispatch Table Generation (classgen.pl)

`core/fw/classgen.pl` processes `core/fw/dev_objs.hpp` (via a C-preprocessed intermediate `dev_objs.tmp` produced by CMake) to generate:

1. **`dev_objs.cgh`** — struct definitions with dispatch table pointers, convenience macro declarations
2. **`dev_objs.c`** — thin wrappers that call through the dispatch table. Generated at build time; not present in the source tree.

```c
br_error ObjectQuery(void *self, void *pvalue, br_token t)
{
    return BR_CMETHOD_CALL(br_object, query, self)((br_object *)self, pvalue, t);
}

br_error RendererPartSet(void *self, br_token part, br_int_32 index, br_token t, br_value value)
{
    return BR_CMETHOD_CALL(br_renderer, partSet, self)((br_renderer *)self, part, index, t, value);
}
```

The `BR_CMETHOD_CALL` macro (`core/inc/compiler.h:307-316`) dereferences `((t *)self)->dispatch->_methodName` to call the driver's implementation. Each driver fills in a dispatch table struct:

```c
// In glrend device.c:
.dispatch = (struct br_device_dispatch){
    ._templateQuery = BR_CMETHOD_REF(br_device_gl, templateQuery),
    // ...
};
```

## Key File Index

| File                          | Purpose                                                                                       |
|-------------------------------|-----------------------------------------------------------------------------------------------|
| `core/inc/pretok.tok`         | Token DSL source — all built-in token names and types                                         |
| `core/inc/tokgen.pl`          | Perl script generating `pretok.h`, `pretok.c`, `toktype.c`                                    |
| `core/inc/token.h`            | `br_token` enum, `br_value` union, `br_token_value` pair                                      |
| `core/fw/ddi/tokenmap.h`      | `br_tv_template_entry`, `br_tv_template`, conversion enums, flags                             |
| `core/fw/token.c`             | `BrTokenCreate`, `BrTokenFind`, `BrTokenType`, `BrTokenSize`                                  |
| `core/fw/tokenval.c`          | `BrTVTemplateAllocate`, `BrTokenValueSet`/`Query`, `templateMakeMap`, `ValueSet`/`ValueQuery` |
| `core/fw/object.c`            | `br_object` query method implementations                                                      |
| `core/fw/dev_objs.hpp`        | C++ class definitions parsed by `classgen.pl` (preprocessed to `dev_objs.tmp` first)          |
| `core/fw/classgen.pl`         | Generates dispatch tables (`dev_objs.cgh`, `dev_objs.c`) and convenience wrappers             |
| `core/inc/compiler.h`         | `BR_CMETHOD_CALL`/`BR_CMETHOD_REF` dispatch macros                                            |
| `drivers/softrend/state.c`    | Softrend part templates, `FindStateTemplate`, `TemplateActions`                               |
| `drivers/softrend/rndstate.c` | Softrend `partSet`/`partQuery` — calls `FindStateTemplate`                                    |
| `drivers/glrend/device.c`     | glrend device template entries (DDI version, OpenGL version)                                  |
| `drivers/glrend1x/pstate.c`   | glrend1x primitive state template entries                                                     |
| `drivers/sdl3dev/glrend.c`    | sdl3dev querying glrend for OpenGL version tokens                                             |
