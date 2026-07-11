# BRender Object Model

## Overview

BRender implements a vtable-based object model in C. Class definitions in
`core/fw/dev_objs.hpp` (a C++ header) are parsed by `core/fw/classgen.pl`
to generate C-compatible structs with dispatch tables. Each "class" is a
C struct whose first field is a pointer to a function-pointer table. Public
API wrappers expand macros like `ObjectFree(self)` to
`self->dispatch->_free(self)`.

The model provides: polymorphic virtual methods, token/value property
queries via `br_tv_template`, container-based object management (linked
lists), and a layered device/facility system. It is conceptually similar
to COM or GLib GObject but simpler — no reference counting, no
`IUnknown::QueryInterface`, just flat vtables and type tokens.

All DDI classes inherit from `br_object`. The public API hides the dispatch
table behind convenience functions generated from `dev_objs.hpp`.

## Class Hierarchy

```
br_object
├── br_object_container
│   ├── br_device
│   ├── br_facility
│   │   ├── br_output_facility
│   │   └── br_renderer_facility
│   ├── br_renderer
│   └── br_primitive_library
├── br_device_pixelmap
├── br_device_clut
├── br_renderer_state_stored
├── br_buffer_stored
├── br_primitive_state
└── br_geometry
    ├── br_geometry_stored
    ├── br_geometry_v1_model
    ├── br_geometry_v1_buckets
    └── br_geometry_lighting
```

## Code Generation Pipeline

### Input: `core/fw/dev_objs.hpp`

A C++ header defining classes with virtual methods. Example (simplified):

```cpp
class br_object {
public:
    void *reserved[4];
    virtual void free();
    virtual const char *identifier();
    virtual br_token type();
    virtual br_boolean isType(br_token t);
    virtual br_device *device();
    virtual br_size_t space();
protected:
    virtual br_tv_template *templateQuery();
public:
    virtual br_error query(void *pvalue, br_token t);
    // ...
};

class br_object_container : public br_object {
public:
    virtual br_error addFront(br_object *h);
    virtual br_error remove(br_object *h);
    virtual br_error find(br_object **ph, br_token type,
                          const char *pattern, br_token_value *tv);
    // ...
};
```

### Generator: `core/fw/classgen.pl`

CMake first runs the C preprocessor on `dev_objs.hpp` to produce
`dev_objs.tmp`, which `classgen.pl` then processes to generate:

1. **`dev_objs.cgh`** — Struct forward declarations, dispatch table structs
   (flat — all ancestor slots included), and public convenience macros.

2. **`dev_objs_gen.c`** — Thin wrapper functions that call through the
   dispatch table. For example, `ObjectFree` expands to:

   ```c
   void ObjectFree(void *self) {
       ((br_object *)self)->dispatch->_free((br_object *)self);
   }
   ```

### Dispatch Macros

Defined in `core/inc/compiler.h`:

```c
#define BR_CMETHOD_DECL(t, m)    BR_METHOD _M_##t##_##m
#define BR_CMETHOD_REF(t, m)     (_M_##t##_##m)
#define BR_CMETHOD_CALL(t, m, o) (((t *)(o))->dispatch->_##m)
```

A call like `ObjectFree(ptr)` expands to:
```c
((br_object *)(ptr))->dispatch->_free((br_object *)(ptr));
```

### Private Struct Pattern

The generated `dev_objs.cgh` wraps each struct typedef in guards:

```c
#ifndef BR_DEVICE_PRIVATE
typedef struct br_device {
    struct br_device_dispatch *dispatch;
} br_device;
#endif
```

Drivers `#define BR_DEVICE_PRIVATE` before including to provide their own
struct definition with additional fields:

```c
#define BR_DEVICE_PRIVATE
typedef struct br_device {
    const struct br_device_dispatch *dispatch;
    const char *identifier;
    void *object_list;
    void *res;
    struct device_templates templates;
    struct br_output_facility *output_facility;
    struct br_renderer_facility *renderer_facility;
} br_device;
```

The public header sees only the dispatch pointer; driver internals see the
full struct.

## `br_object` — Root Base Class

**File:** `core/fw/object.c`, defined in `dev_objs.hpp:46`.

Every DDI object is a `br_object`. It provides identity queries and a
token/value property system.

### Core Methods

| Method         | Purpose                                                                                                          |
|----------------|------------------------------------------------------------------------------------------------------------------|
| `free()`       | Destroy the object and release all resources                                                                     |
| `identifier()` | Return the object's name string                                                                                  |
| `type()`       | Return the object's type token (e.g., `BRT_DEVICE`)                                                              |
| `isType(t)`    | Check if object is of type `t` (inheritance-aware: `BRT_DEVICE` is also `BRT_OBJECT_CONTAINER` and `BRT_OBJECT`) |
| `device()`     | Return the owning `br_device`                                                                                    |
| `space()`      | Return the size of the object in bytes                                                                           |

### Token/Value Query System

`br_object` exposes queryable properties through the token system:

| Method                                     | Purpose                                  |
|--------------------------------------------|------------------------------------------|
| `query(pvalue, t)`                         | Query a single property by token         |
| `queryMany(tv, extra, extra_size, pcount)` | Query multiple properties at once        |
| `queryManySize(pextra_size, tv)`           | Query buffer size needed for `queryMany` |
| `queryAll(buffer, buffer_size)`            | Query all properties                     |
| `queryAllSize(psize)`                      | Query buffer size needed for `queryAll`  |

The protected `templateQuery()` method (pure virtual) returns a
`br_tv_template *` — a lookup table mapping token constants to struct field
offsets. The public `query()` methods delegate to
`BrTokenValueQuery()`/`BrTokenValueQueryMany()` in `core/fw/tokenval.c`,
which use the template to locate and convert values.

Default implementations for `query()`, `queryMany()`, etc. live in
`object.c`. Drivers override `templateQuery()` to expose their own
properties. The query system is what enables code like:

```c
br_value value;
ObjectQuery(device, &value, BRT_OPENGL_VERSION_MAJOR_U8);
```

See `docs/tokens.md` for the full token/template system.

### Type Tokens

Each DDI class has a type token from `pretok.h`:

| Class | Token | Value |
|-------|-------|-------|
| `br_object` | `BRT_OBJECT` | 19 |
| `br_object_container` | `BRT_OBJECT_CONTAINER` | 42 |
| `br_device` | `BRT_DEVICE` | 43 |
| `br_output_facility` | `BRT_OUTPUT_FACILITY` | 45 |
| `br_device_pixelmap` | `BRT_DEVICE_PIXELMAP` | 46 |
| `br_renderer_facility` | `BRT_RENDERER_FACILITY` | 48 |
| `br_renderer` | `BRT_RENDERER` | 49 |
| `br_primitive_library` | `BRT_PRIMITIVE_LIBRARY` | 58 |
| `br_primitive_state` | `BRT_PRIMITIVE_STATE` | 59 |

### Object Type Hierarchy

`isType()` supports inheritance: a `br_device` reports true for
`BRT_DEVICE`, `BRT_OBJECT_CONTAINER`, and `BRT_OBJECT`. This is how
`ObjectContainerFind()` can search for all objects matching
`BRT_OUTPUT_FACILITY` by walking containers and checking `isType()`.

## `br_object_container` — Container for Objects

**Parent:** `br_object`
**File:** `core/fw/objectc.c`, defined in `dev_objs.hpp:77`.

Extends `br_object` with linked-list container semantics. Objects can be
added, removed, and searched by type, name pattern, and token/value
filters.

### Container Methods

| Method                                            | Purpose                                                               |
|---------------------------------------------------|-----------------------------------------------------------------------|
| `addFront(h)`                                     | Add object to front of the internal list                              |
| `remove(h)`                                       | Remove a specific object                                              |
| `removeFront(ph)`                                 | Remove and return the front object                                    |
| `find(ph, type, pattern, tv)`                     | Find first object matching type token, name glob, and property values |
| `findMany(objects, max, pnum, type, pattern, tv)` | Find all matching objects                                             |
| `count(pcount, type, pattern, tv)`                | Count matching objects                                                |

### Internal Implementation

The container manages a `BR_SIMPLELIST` via `objectc.c`:

```c
struct object_list      { br_simple_list l; };
struct object_list_entry { br_simple_node n; br_object *h; };
```

Each entry wraps a `br_object *`. The list is allocated via
`BrObjectListAllocate(res)` on first use and freed when the container is
destroyed.

Container semantics form the backbone of the device model: **a device
contains its facilities, a facility contains its renderer instances and
geometry formats**. This parent-child relationship also controls resource
lifetime — freeing a container frees all children.

### Matching Objects

`find()` / `findMany()` / `count()` filter by:
- **type token** — matches via `isType()`, so `BRT_FACILITY` matches both
  `br_output_facility` and `br_renderer_facility`
- **name pattern** — glob-style pattern (e.g., `"OpenGL*"`)
- **token/value pairs** — property filters (e.g., find all pixelmaps with
  width ≥ 640)

```c
ObjectContainerFind(device, &facility, BRT_OUTPUT_FACILITY, "OpenGL", NULL);
```

This finds the `br_output_facility` named "OpenGL" registered inside
`device`.

## `br_device` — The Device

**Parent:** `br_object_container`
**File:** Defined in `dev_objs.hpp:98`, implemented per-driver (e.g.,
`drivers/glrend/device.c`).

The top-level rendering device. Loaded from shared libraries, registered
in the framework's device table, and serves as the resource anchor for all
driver allocations.

### No Additional Virtual Methods

`br_device` inherits all methods from `br_object_container` and adds none.
It exists primarily to hold:
- The resource anchor (`self->res` — all driver allocations are children of
  this)
- The output facility (creates pixelmaps/surfaces)
- The renderer facility (creates renderer instances)
- Any other contained objects (primitive libraries, etc.)

### Device Lifecycle

1. **Loading**: `BrDevAdd(&dev, "glrend", NULL)` calls the shared library's
   `BrDrv1Begin()` entry point, which allocates the device struct, creates
   facilities, and registers them with `ObjectContainerAddFront()`.

2. **Registration**: The returned `br_device *` is stored in
   `fw.dev_slots[]`, an array of all loaded devices.

3. **Lookup**: `BrDevFind(&dev, "OpenGL")` scans `fw.dev_slots[]` by
   name pattern.

4. **Destruction**: `BrDevRemove(dev)` calls `ObjectFree(dev)`, which
   cascades through all contained objects via the resource system.

Implementation in `core/fw/devlist.c`.

## `br_facility` — Abstract Facility

**Parent:** `br_object_container`
**File:** Defined in `dev_objs.hpp:104`.

An abstract base representing a service capability that a device offers.
Extends `br_object_container` with no additional methods. Two concrete
types exist: `br_output_facility` and `br_renderer_facility`.

The container nature of facilities means they can hold their own children.
For example, a `br_renderer_facility` contains its geometry format objects
(`br_geometry_v1_buckets`, `br_geometry_lighting`,
`br_geometry_v1_model`), and each renderer instance it creates gets
registered as a child.

## `br_output_facility` — Screen / Output Mode Facility

**Parent:** `br_facility` → `br_object_container` → `br_object`
**File:** Defined in `dev_objs.hpp:113`, implemented per-driver (e.g.,
`drivers/glrend/outfcty.c`).

Manages display output and surface creation. Each device has one output
facility registered as a child.

### Methods

| Method                                         | Purpose                                                  |
|------------------------------------------------|----------------------------------------------------------|
| `validSource(bp, h)`                           | Check if an object can be used as a render source        |
| `pixelmapNew(ppmap, tv)`                       | Create a new `br_device_pixelmap` from token/value pairs |
| `clutNew(pclut, tv)`                           | Create a new `br_device_clut`                            |
| `queryCapability(buffer_in, buffer_out, size)` | Query capability information                             |

### Creating a Pixelmap

`pixelmapNew()` is the primary factory for drawable surfaces. It accepts
token/value pairs specifying width, height, pixel type, etc.:

```c
br_token_value tv[] = {
    {BRT_WIDTH_I32,  {.i32 = 640}},
    {BRT_HEIGHT_I32, {.i32 = 480}},
    {BRT_TYPE_U8,    {.u8  = BR_PMT_RGB_888}},
    {BR_NULL_TOKEN,  {.i32 = 0}},
};
OutputFacilityPixelmapNew(output_facility, &pm, tv);
```

The output facility implementation allocates the `br_device_pixelmap`,
sets its dispatch table, initialises the platform-specific surface (GL
framebuffer, SDL window, DOS VESA mode, etc.), and returns it.

### Discovery

The public API `BrDevBegin()` combines device lookup with output facility
discovery:

```c
BrDevBegin(&screen, "OpenGL");
```

This finds the device with an output facility named "OpenGL", creates a
default pixelmap from that facility, and returns it as a `br_pixelmap *`.

Implementation in `core/fw/devsetup.c`.

## `br_renderer_facility` — Renderer Facility

**Parent:** `br_facility` → `br_object_container` → `br_object`
**File:** Defined in `dev_objs.hpp:254`, implemented per-driver (e.g.,
`drivers/glrend/rendfcty.c`).

Creates renderer instances and validates render destinations.

### Methods

| Method | Purpose |
|--------|---------|
| `validDestination(bp, h)` | Check if an object is a valid render target |
| `rendererNew(prenderer, tv)` | Create a new `br_renderer` instance |

### Children

A renderer facility creates geometry format objects as children:

```c
ObjectContainerAddFront(facility, geometry_v1_buckets);
ObjectContainerAddFront(facility, geometry_lighting);
ObjectContainerAddFront(facility, geometry_v1_model);
```

These are discovered by the V1 database layer via
`ObjectContainerFind(facility, ..., BRT_GEOMETRY_V1_MODEL, ...)`.

### Finding a Renderer Facility

`BrRendererFacilityFind()` in `core/fw/devsetup.c` implements a multi-step
discovery process:

1. Query destination pixelmap's `BRT_RENDERER_FACILITY_O` token
2. Query the output facility for its default renderer
3. Search loaded devices for `"Default-Renderer-*"` facilities
4. Load `softrend` as a fallback and search again
5. Search for `"Default-Renderer"` as a last resort

## `br_renderer` — Active Renderer Instance

**Parent:** `br_object_container` → `br_object`
**File:** Defined in `dev_objs.hpp:262`, implemented per-driver (e.g.,
`drivers/glrend/renderer.c`).

The renderer is the workhorse of the DDI. It has the largest dispatch
table (~40+ methods), covering state management, geometry submission,
render control, and GPU synchronisation.

### State Management

| Method                                           | Purpose                                                    |
|--------------------------------------------------|------------------------------------------------------------|
| `partSet(part, index, t, value)`                 | Set a single value in a state part (surface, matrix, etc.) |
| `partSetMany(part, index, tv, pcount)`           | Set multiple values at once                                |
| `partQuery(part, index, pvalue, t)`              | Query a single value                                       |
| `partQueryMany(part, index, tv, extra, pcount)`  | Query multiple values                                      |
| `partQueryAll(part, index, buffer, buffer_size)` | Query all values in a part                                 |
| `partIndexQuery(part, pnindex)`                  | Query the index range for a part                           |
| `statePush(mask)` / `statePop(mask)`             | Push/pop parts of renderer state                           |
| `stateSave(h, mask)` / `stateRestore(h, mask)`   | Save/restore state to/from a `br_renderer_state_stored`    |
| `stateDefault(mask)`                             | Reset parts to default values                              |
| `stateMask(pmask, parts, n_parts)`               | Convert state part tokens to a mask                        |

State parts include: `BRT_SURFACE` (material properties), `BRT_MATRIX`
(transforms), `BRT_CULL` (culling), `BRT_ENABLE` (toggles),
`BRT_HIDDEN_SURFACE` (z-sort mode), `BRT_BOUNDS`, `BRT_LIGHT`,
`BRT_CLIP`. See `docs/rendering.md` for the rendering pipeline.

### Transform Helpers

| Method | Purpose |
|--------|---------|
| `modelMul(m)` | Multiply the current model matrix by `m` |
| `modelPopPushMul(m)` | Pop, convert to current type, multiply, push |
| `modelInvert()` | Invert the current model matrix |

### Stored Object Creation

| Method                              | Purpose                                      |
|-------------------------------------|----------------------------------------------|
| `stateStoredNew(pstate, mask, tv)`  | Create a `br_renderer_state_stored` snapshot |
| `stateStoredAvail(psize, mask, tv)` | Query space needed for stored state          |
| `bufferStoredNew(rtp, use, pm, tv)` | Create a `br_buffer_stored` from a pixelmap  |
| `bufferStoredAvail(space, use, tv)` | Query space needed for stored buffer         |

### Render Control

| Method                                    | Purpose                           |
|-------------------------------------------|-----------------------------------|
| `frameBegin()` / `frameEnd()`             | Bracket a single rendered frame   |
| `sceneBegin()` / `sceneEnd()`             | Bracket a scene within a frame    |
| `flush(wait)`                             | Flush pending rendering commands  |
| `synchronise(sync_type, block)`           | Synchronise with GPU              |
| `focusLossBegin()` / `focusLossEnd()`     | Handle focus/context loss         |
| `boundsTest(r, bounds)`                   | Test if bounds intersect the view |
| `commandModeSet()` / `commandModeQuery()` | Configure command recording mode  |

### glrend Private Struct

```c
typedef struct br_renderer {
    const struct br_renderer_dispatch *dispatch;
    const char *identifier;
    struct br_device *device;
    void *object_list;
    const GladGLContext *gl;
    br_device_pixelmap *pixelmap;
    br_renderer_facility *renderer_facility;
    br_pool *state_pool;
    state_all state;
    br_hashmap *sampler_pool;
    br_buffer_ring_gl model_ring;     // UBO ring buffer
    int has_begun;
} br_renderer;
```

### Creation

```c
br_renderer *RendererGLAllocate(br_device *device,
    br_renderer_facility *facility, br_device_pixelmap *dest);
```

Allocates the renderer from the facility's resource pool, sets up the GL
context, creates the immediate-mode VAO/VBO for non-stored geometry,
initialises default state, and registers itself as a child of the
facility.

## `br_device_pixelmap` — Drawable Surface

**Parent:** `br_object` (not a container)
**File:** Defined in `dev_objs.hpp:125`, implemented in
`core/pixelmap/pmmem.c` and per-driver.

`br_device_pixelmap` is the DDI drawable surface. It includes public fields
(via `CG_PUBLIC` in `dev_objs.hpp`) that make its memory layout compatible
with the public `br_pixelmap` struct.

### The `br_pixelmap` / `br_device_pixelmap` Relationship

**Every pixelmap in BRender is internally a `br_device_pixelmap`.** The
public `br_pixelmap` is a layout-compatible view of the same memory:

```c
// dev_objs.cgh — generated:
typedef struct br_device_pixelmap {
    struct br_device_pixelmap_dispatch *dispatch;
    char *pm_identifier;
    BR_PIXELMAP_MEMBERS       // pixels, type, width, height, row_bytes, ...
} br_device_pixelmap;

// pixelmap.h — public:
typedef struct br_pixelmap {
    br_uintptr_t _reserved;   // occupies same offset as dispatch
    char *identifier;
    BR_PIXELMAP_MEMBERS
} br_pixelmap;
```

The `_reserved` field at offset 0 occupies the same bytes as the
`dispatch` pointer. A "memory pixelmap" — one not created by an output
facility — initially has `dispatch == NULL`. The function
`_CheckDispatch()` in `pmmem.c` detects this and fills in the default
memory dispatch table lazily:

```c
void _CheckDispatch(br_device_pixelmap *pm)
{
    if(pm->dispatch == NULL)
        pm->dispatch = &devicePixelmapDispatch;
}
```

This means code can freely pass `br_pixelmap *` where `br_device_pixelmap *`
is expected, and vice versa, via cast.

### Methods — Whole Pixelmap Operations

| Method                                        | Purpose                      |
|-----------------------------------------------|------------------------------|
| `copy(dst)` / `copyTo(dst)` / `copyFrom(src)` | Copy entire pixelmaps        |
| `fill(colour)`                                | Fill with a solid colour     |
| `doubleBuffer(src)`                           | Swap display buffers         |
| `flush()`                                     | Flush pending output         |
| `synchronise(sync_type, block)`               | GPU synchronisation          |
| `directLock(block)` / `directUnlock()`        | Lock for direct pixel access |

### Methods — 2D Drawing

| Method                                                | Purpose           |
|-------------------------------------------------------|-------------------|
| `rectangle(r)` / `rectangle2(r)` / `rectangleFill(r)` | Rectangle drawing |
| `rectangleCopy(dst, sr, dr)`                          | Rectangle blit    |
| `rectangleStretchCopy(dst, sr, dr)`                   | Stretched blit    |
| `pixelSet(p, colour)` / `pixelQuery(p, pcolour)`      | Point operations  |
| `line(s, e)`                                          | Line drawing      |
| `copyBits(src, sr, dr)`                               | Bitmap copy       |
| `text(p, str, font)` / `textBounds(str, font, pr)`    | Text rendering    |

### Methods — Data Access

| Method                          | Purpose                   |
|---------------------------------|---------------------------|
| `rowSize(row)`                  | Query row size in bytes   |
| `rowSet(row, data)`             | Write a whole row         |
| `rowQuery(row, data)`           | Read a whole row          |
| `pixelQuery(p, pcolour)`        | Read a single pixel       |
| `pixelAddressQuery(p, ppixels)` | Get pointer to pixel data |
| `pixelAddressSet(p, pixels)`    | Set pixel data pointer    |
| `originSet(p)`                  | Set origin offset         |

### Methods — Control

| Method                   | Purpose                     |
|--------------------------|-----------------------------|
| `getControls(vp, pc)`    | Query display controls      |
| `setControls(vp, c)`     | Set display controls        |
| `handleWindowEvent(arg)` | Window event (resize, etc.) |

## `br_device_clut` — Colour Lookup Table

**Parent:** `br_object`
**File:** Defined in `dev_objs.hpp:242`.

A hardware colour lookup table for indexed-colour modes. Maps palette
indices to RGB colours.

| Method                                  | Purpose                 |
|-----------------------------------------|-------------------------|
| `entrySet(index, colour)`               | Set a single CLUT entry |
| `entryQuery(pcolour, index)`            | Query a single entry    |
| `entrySetMany(index, count, entries)`   | Set multiple entries    |
| `entryQueryMany(entries, index, count)` | Query multiple entries  |

Created by `br_output_facility::clutNew()`.

## `br_renderer_state_stored` — Cached Renderer State

**Parent:** `br_object`
**File:** Defined in `dev_objs.hpp:354`.

An opaque snapshot of renderer state. Has no public methods beyond
`br_object`. Created by `RendererStateStoredNew()` and updated by
`RendererStateSave()`.

### Relationship to `br_material`

**`material->stored` is a `br_renderer_state_stored *`.** When a material
is prepared (`prepmatl.c`), its surface properties (ambient, diffuse,
specular, opacity), primitive settings (blend mode, texture, fog), and
culling mode are translated into renderer state tokens and captured in a
stored state object. The material stores this handle so it can be rapidly
bound at render time without re-translating all the material flags.

```c
// prepmatl.c — during material preparation:
RendererStatePush(...);                         // save current renderer state
RendererStateDefault(PARTS_MASK);               // reset surface/primitive/cull to defaults
if(mat->stored)
    RendererStateRestore(mat->stored, mask);    // restore previous stored state as base
// Translate material fields → RendererPartSet() calls...
if(mat->stored)
    RendererStateSave(mat->stored, PARTS_MASK); // overwrite existing stored state
else
    RendererStateStoredNew(&mat->stored, ...);  // create new stored state
RendererStatePop(...);                          // restore original renderer state
```

The `PARTS_MASK` is `BR_STATE_SURFACE | BR_STATE_PRIMITIVE | BR_STATE_CULL`
— the parts of renderer state that a material controls.

At render time, `material->stored` is passed to geometry rendering as the
default state for faces with no per-face material override. This is
analogous to a Vulkan pipeline state object or a D3D12 Pipeline State
Object — a pre-baked configuration bound with a single operation.

## `br_buffer_stored` — GPU-side Pixelmap Resource

**Parent:** `br_object`
**File:** Defined in `dev_objs.hpp:408`.

Represents pixelmap data uploaded to the GPU or otherwise prepared for
device use. Has a single method:

| Method           | Purpose                                                |
|------------------|--------------------------------------------------------|
| `update(pm, tv)` | Update the stored buffer from a `br_device_pixelmap *` |

### Relationship to `br_pixelmap`

**`pixelmap->stored` is a `br_buffer_stored *`.** When `BrMapUpdate()` or
`BrBufferUpdate()` is called on a pixelmap, the renderer creates a stored
buffer wrapping the pixelmap data:

```c
// prepmap.c:
void BrBufferUpdate(br_pixelmap *pm, br_token use, br_uint_16 flags) {
    if(pm->stored)
        BufferStoredUpdate(pm->stored, (br_device_pixelmap *)pm, tv);
    else
        RendererBufferStoredNew(v1db.renderer, (br_buffer_stored **)&pm->stored,
                                use, (br_device_pixelmap *)pm, tv);
}
```

Note the cast from `br_pixelmap *` to `br_device_pixelmap *` — this works
because of the layout compatibility described above.

The `use` token (e.g., `BRT_COLOUR_MAP_O`, `BRT_INDEX_SHADE_TABLE_O`,
`BRT_INDEX_BLEND_TABLE_O`) tells the renderer how to create the GPU resource
— as a texture, a shade table, a blend table, etc.

When setting a texture on a material, the stored buffer is what gets bound:

```c
// prepmatl.c:
tvp->t   = BRT_COLOUR_MAP_O;
tvp->v.o = mat->colour_map ? mat->colour_map->stored : NULL;
```

The `stored` buffer is the GPU-side handle; its `update()` method lets the
renderer refresh the GPU resource when the CPU pixel data changes
(e.g., after procedural generation or file loading).

## `br_geometry` and Subclasses

**Parent:** `br_object`
**File:** Defined in `dev_objs.hpp:360-406`.

`br_geometry` is an abstract base for geometry format renderers. Each
subclass provides a different way to render geometric data.

### `br_geometry_stored`

**Defined in:** `dev_objs.hpp:366`

Renders pre-uploaded GPU geometry (VBO-based in glrend).

| Method                    | Purpose                                   |
|---------------------------|-------------------------------------------|
| `render(r, type)`         | Render stored geometry with `br_renderer` |
| `renderOnScreen(r, type)` | Render in screen space                    |

**`model->stored` is a `br_geometry_stored *`.** When `BrModelUpdate()`
prepares a model, it also creates a stored geometry from the prepared
vertex data via `GeometryV1ModelStoredNew()`. At render time, if
`model->stored` exists, `GeometryStoredRender()` is used directly; if not,
the renderer falls back to `GeometryV1ModelRender()` with the prepared
data and material state.

### `br_geometry_v1_model`

**Defined in:** `dev_objs.hpp:376`

Renders V1 model prepared data (`v11model`) through the renderer's CPU
transform and lighting pipeline.

| Method                                          | Purpose                                  |
|-------------------------------------------------|------------------------------------------|
| `render(r, model, default_state, type)`         | Render a `v11model` using `br_renderer`  |
| `renderOnScreen(r, model, default_state, type)` | Screen-space render                      |
| `storedNew(r, psg, model, type, tv)`            | Create `br_geometry_stored` from a model |
| `storedAvail(psize, tv)`                        | Query space needed for stored geometry   |

The `default_state` parameter is `material->stored` — the renderer state
derived from the material.

### `br_geometry_v1_buckets`

**Defined in:** `dev_objs.hpp:390`

Renders primitive buckets from the z-sort path. Used after diverting
blended geometry to the sort heap.

| Method                                 | Purpose                         |
|----------------------------------------|---------------------------------|
| `render(r, buckets, nbuckets)`         | Render sorted primitive buckets |
| `renderOnScreen(r, buckets, nbuckets)` | Screen-space bucket render      |

### `br_geometry_lighting`

**Defined in:** `dev_objs.hpp:398`

Used exclusively by `BrSceneModelLight()` in `core/v1db/prelight.c` for
offline prelighting of model vertices — computing and storing Gouraud
lighting colours into vertex data before rendering starts. Not used during
normal frame rendering (the renderer does its own lighting internally).

Only softrend provides a real implementation (`drivers/softrend/lighting.c`).
glrend registers the null device stub (`GeometryLightingNullAllocate` from
`core/nulldev/glighting.c`) which does nothing — glrend has no concept of
offline prelighting since all lighting runs on the GPU.

| Method                                                                            | Purpose                     |
|-----------------------------------------------------------------------------------|-----------------------------|
| `render(r, points, normals, colour_in, colour_out, redirect, strides, nvertices)` | Compute per-vertex lighting |

### Rendering Path

```
BrActorRender() → renderAll(model, material, type, on_screen) {
    if(model->stored)
        GeometryStoredRender(model->stored, renderer, type);
    else if(model->prepared)
        GeometryV1ModelRender(format_model, renderer, model->prepared,
                              material->stored, type);
}
```

If `model->stored` exists, rendering goes straight to the GPU via
`br_geometry_stored`. Otherwise, the CPU T&L path processes the prepared
model data and applies the material's stored state.

## `br_primitive_library` and `br_primitive_state`

### `br_primitive_library`

**Parent:** `br_object_container`
**File:** Defined in `dev_objs.hpp:415`.

The software rasterizer library (softrend's primitives layer, or pentprim).
Creates primitive state objects and stored buffers, flushes rasterisation,
and synchronises.

| Method                              | Purpose                           |
|-------------------------------------|-----------------------------------|
| `stateNew(prps)`                    | Create a new `br_primitive_state` |
| `bufferStoredNew(rtp, use, pm, tv)` | Create a stored buffer            |
| `bufferStoredAvail(space, use, tv)` | Query buffer space                |
| `flush(wait)`                       | Flush rasterisation               |
| `synchronise(sync_type, block)`     | Synchronise                       |
| `mask(pmask, parts, n_parts)`       | Convert part tokens to mask       |

### `br_primitive_state`

**Parent:** `br_object`
**File:** Defined in `dev_objs.hpp:434`.

Mirrors `br_renderer`'s state API but for the software rasterizer layer.
Controls rasteriser configuration: colour mode, depth test, blend flags,
texture binding, component ranges.

| Method                                 | Purpose                                      |
|----------------------------------------|----------------------------------------------|
| `partSet()` / `partQuery()` / etc.     | State read/write (same pattern as renderer)  |
| `stateDefault(mask)`                   | Reset to defaults                            |
| `stateCopy(source, mask)`              | Copy state from another `br_primitive_state` |
| `renderBegin(rpb, ...)`                | Begin a primitive rendering block            |
| `renderEnd(rpb)`                       | End a rendering block                        |
| `rangesQuery(offset, scale, max_comp)` | Query component output ranges                |

The `renderBegin()`/`renderEnd()` pair brackets primitive block submission.
This is how `glrend1x` receives softrend's computed vertices — softrend
calls through the `br_primitive_state` dispatch which glrend1x hooks to
issue its own `glBegin`/`glEnd`/`glVertex*` calls.

## Dispatch Table Pattern

### Flat Vtable

Dispatch tables are flat arrays of function pointers. A subclass dispatch
struct includes all ancestor slots at the same indices. For example:

```c
struct br_device_dispatch {
    // br_object slots (indices 0–16)
    void         (*_reserved0)(void);
    void         (*_reserved1)(void);
    void         (*_reserved2)(void);
    void         (*_reserved3)(void);
    br_error     (*_free)(br_object *);
    const char * (*_identifier)(br_object *);
    br_token     (*_type)(br_object *);
    // ... 11 more br_object methods ...
    br_error (*_queryBuffer)(br_object *, void *, void *, br_size_t, br_token);
    // br_object_container slots (indices 17–25)
    br_error   (*_addFront)(br_object_container *, br_object *);
    br_error   (*_remove)(br_object_container *, br_object *);
    br_error   (*_removeFront)(br_object_container *, br_object **);
    // ... 6 more container methods ...
};
```

Calling `ObjectContainerAddFront(device, obj)` dispatches to
`device->dispatch->_addFront(device, obj)` — same function pointer at the
same vtable index regardless of whether the device is glrend or softrend.

### Default Implementations

The framework provides default method implementations in
`core/fw/object.c` and `core/fw/objectc.c`. Drivers include these in their
dispatch table or override with driver-specific versions.

For example, `object.c` provides:

```c
br_error _BrObjectFree(br_object *self) { ... }
const char * _BrObjectIdentifier(br_object *self) { return self->identifier; }
br_token _BrObjectType(br_object *self) { ... }
br_error _BrObjectQuery(br_object *self, void *pvalue, br_token t) { ... }
// etc.
```

And `objectc.c` provides:

```c
br_error _BrObjectContainerAddFront(br_object_container *self, br_object *h) { ... }
br_error _BrObjectContainerRemove(br_object_container *self, br_object *h) { ... }
br_error _BrObjectContainerFind(br_object_container *self, br_object **ph, ...) { ... }
```

### Driver Dispatch Table Assembly

Drivers fill in the dispatch table by referencing the default
implementations or their own:

```c
// glrend device.c:
static const struct br_device_dispatch deviceDispatch = {
    ._reserved0    = (void *)0,
    // ...
    ._free         = BR_CMETHOD_REF(br_object, free),
    ._identifier   = BR_CMETHOD_REF(br_object, identifier),
    ._type         = BR_CMETHOD_REF(br_object, type),
    ._isType       = BR_CMETHOD_REF(br_object, isType),
    ._device       = BR_CMETHOD_REF(br_object, device),
    ._space        = BR_CMETHOD_REF(br_device_gl, space),      // override
    ._templateQuery = BR_CMETHOD_REF(br_device_gl, templateQuery), // override
    ._query        = BR_CMETHOD_REF(br_object, query),
    // ... container methods ...
    ._addFront     = BR_CMETHOD_REF(br_object_container, addFront),
    ._remove       = BR_CMETHOD_REF(br_object_container, remove),
    // ...
};

br_device *device = BrResAllocate(res, sizeof(br_device), BR_MEMORY_OBJECT);
device->dispatch = &deviceDispatch;
```

`BR_CMETHOD_REF(class, method)` resolves to `_M_class_method`, which is
the C name of the function.

## Object Lifecycle

### Allocation

Objects are allocated via `BrResAllocate()`:

```c
br_device *device = BrResAllocate(
    fw.res,                     // parent resource
    sizeof(br_device),          // allocation size
    BR_MEMORY_OBJECT            // resource class
);
```

The `parent` argument chains the allocation to a resource tree. The
framework's root resource anchor is `fw.res`. Each device has its own
`self->res` anchor, so all driver allocations are children of the device.

### Dispatch Assignment

Immediately after allocation, the dispatch pointer is set:

```c
device->dispatch = &deviceDispatch;
```

### Registration

Containers register children so they appear in searches and get cleaned up
automatically:

```c
ObjectContainerAddFront(device, (br_object *)output_facility);
```

### Destruction

`ObjectFree(obj)` calls `obj->dispatch->_free(obj)`, which is typically
the resource destructor `_BrObjectFree()`. This frees the resource anchor,
which cascades to all child allocations — destroying every object,
facility, renderer, and buffer that was allocated under that anchor.

```c
ObjectFree(device);  // destroys device, facilities, renderers, everything
```

## The "stored" Pattern

The V1 database layer uses a consistent pattern where each CPU-side type
has a `stored` pointer to its device-side counterpart:

| CPU-side field     | Device-side type             | Purpose                                           | Created in   |
|--------------------|------------------------------|---------------------------------------------------|--------------|
| `material->stored` | `br_renderer_state_stored *` | Pre-baked pipeline state from material properties | `prepmatl.c` |
| `pixelmap->stored` | `br_buffer_stored *`         | GPU texture/shade-table from pixel data           | `prepmap.c`  |
| `model->stored`    | `br_geometry_stored *`       | GPU VBO geometry from model vertices              | `prepmesh.c` |

All three fields are declared `void *` in the public headers but hold DDI
object pointers. The cast is implicit because `stored` is opaque to the V1
layer — only the renderer facility knows how to create, use, and free
these objects.

This is BRender's equivalent of modern "resource binding": the CPU-side data
(material, pixelmap, model) gets an opaque device-side handle that the
renderer understands natively. Once created, rendering can bind the GPU
resource without re-translating the CPU-side data.

### What is NOT stored

`br_actor->render_data` is **not** a stored DDI object. It holds an
`br_order_table *` (for the z-sort path) or `NULL` (for the z-buffer
path). It is raw renderer-specific data, not a managed `br_object`
subclass.

## Relationship to the V1 Database

Actors (`br_actor`), models (`br_model`), and materials (`br_material`)
are plain C structs — they have no vtable, no dispatch table, and do not
inherit from `br_object`. They live in the V1 database layer
(`core/v1db/`) and are bridged to the DDI object model at render time:

```
br_actor (tree of transforms + model refs + material refs)
    │
    ▼  actorRender()
br_model → prepmesh.c → br_geometry_stored (model->stored)
br_material → prepmatl.c → br_renderer_state_stored (material->stored)
br_pixelmap → prepmap.c → br_buffer_stored (pixelmap->stored)
    │
    ▼  renderAll()
GeometryStoredRender() or GeometryV1ModelRender()
    │
    ▼
br_renderer → submit to GPU / software rasterizer
```

The V1 database is the scene graph and asset storage. The DDI object model
is the device abstraction layer. The "stored" pattern is the bridge
between them.

## Key File Index

| File                        | Purpose                                                                   |
|-----------------------------|---------------------------------------------------------------------------|
| `core/fw/dev_objs.hpp`      | C++ class definitions — input to `classgen.pl`                            |
| `core/fw/classgen.pl`       | Generates dispatch tables and convenience wrappers                        |
| `core/fw/object.c`          | `br_object` default method implementations                                |
| `core/fw/objectc.c`         | `br_object_container` list management                                     |
| `core/fw/tokenval.c`        | Token/value query/set machinery                                           |
| `core/fw/devlist.c`         | Device loading/unloading (`BrDevAdd`/`Find`/`Remove`)                     |
| `core/fw/devsetup.c`        | `BrDevBegin()`, `BrRendererFacilityFind()`                                |
| `core/fw/ddi/tokenmap.h`    | `br_tv_template_entry`, conversion enums, flags                           |
| `core/inc/compiler.h`       | `BR_CMETHOD_CALL`/`BR_CMETHOD_REF`/`BR_CMETHOD_DECL`                      |
| `core/inc/token.h`          | `br_value` union, `br_token_value` pair                                   |
| `core/inc/pixelmap.h`       | Public `br_pixelmap` struct (layout-compatible with `br_device_pixelmap`) |
| `core/pixelmap/pmmem.c`     | Default memory `br_device_pixelmap` dispatch and `_CheckDispatch()`       |
| `core/inc/actor.h`          | `br_actor` definition (V1 scene graph — NOT a `br_object`)                |
| `core/inc/model.h`          | `br_model` definition                                                     |
| `core/inc/material.h`       | `br_material` definition                                                  |
| `core/v1db/prepmatl.c`      | Material → `br_renderer_state_stored` translation                         |
| `core/v1db/prepmap.c`       | Pixelmap → `br_buffer_stored` upload                                      |
| `core/v1db/prepmesh.c`      | Model → `br_geometry_stored` creation                                     |
| `core/v1db/modrend.c`       | `renderAll()` — model rendering dispatch                                  |
| `drivers/glrend/device.c`   | glrend device dispatch table                                              |
| `drivers/glrend/device.h`   | glrend device private struct                                              |
| `drivers/glrend/outfcty.c`  | glrend output facility                                                    |
| `drivers/glrend/rendfcty.c` | glrend renderer facility                                                  |
| `drivers/glrend/renderer.c` | glrend renderer implementation                                            |
| `drivers/glrend/renderer.h` | glrend renderer private struct                                            |
