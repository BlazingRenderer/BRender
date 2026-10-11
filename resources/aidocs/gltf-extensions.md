# glTF extensions

This document specifies the three `BR_`-prefixed glTF 2.0 extensions the BRender glTF reader and writer in
this repository carry - `BR_actors`, `BR_lights` and `BR_materials` (`BrFmtGLTFActorLoadMany()` in
`core/fmt/loadgltf.c`, `BrFmtGLTFActorSaveMany()` in `core/fmt/savegltf.c`) - together with the marker the
writer puts into an image's data URI (`brender=index8|rgb555|rgb565|rgb888`). `BR_actors` and the URI marker
are this fork's; `BR_lights` and `BR_materials` come from the glTF work that predates it, and are in
upstream too. **None of the three is submitted to, proposed to, or registered with the Khronos glTF registry,
and no other glTF implementation is expected to understand any of it**: what follows is normative for the
implementation in this repository and nothing else, which is why the extensions are `BR_`-prefixed rather
than `KHR_`-prefixed. They exist to carry state that glTF 2.0 has no field for.

Where this document and the code disagree, the code is what ships.

## Where each piece lives

| piece | file |
| --- | --- |
| the extension structs and the URI markers | `core/fmt/cgltf_brender.h` |
| parsing (`BR_*` object properties, and the defaults for absent ones) | `core/fmt/cgltf_brender_impl.h` |
| the extension-name dispatch on nodes, primitives and the root | `core/fmt/cgltf.h` |
| writing the property values | `core/fmt/cgltf_write_brender.h` |
| where each extension object is written | `core/fmt/cgltf_write.h` |
| converting to and from `br_*` objects | `core/fmt/loadgltf.c`, `core/fmt/savegltf.c` |

`cgltf.h` and `cgltf_write.h` are vendored cgltf with these additions in them; the additions are kept in the
`cgltf_*_brender.*` files where possible, so the vendored files stay easy to update.

## Conventions that hold for all three extensions

* **Every property is optional.** An absent property reads as the default in the field table, and the
  defaults are the same on the reading and the writing side, so a property that is omitted reads back as what
  was written. The writer omits any property whose value *is* the default (`cgltf_write_intprop()`,
  `cgltf_write_floatprop()`, `cgltf_write_boolprop_optional()` and `cgltf_check_floatarray()` in
  `core/fmt/cgltf_write.h`), which is what makes an "absent means default" table sufficient.
* **A root extension object holds an array; a node or primitive names an entry by index.** The index is into
  that extension's own root array (`extensions.BR_materials.materials`, `extensions.BR_lights.lights`), not
  into glTF's `materials` array or `KHR_lights_punctual`'s `lights` array.
* **`BR_actors` is the exception**: its value sits in place on the node, with no root array and no index.
* **Colours are three floats in 0..1, RGB only.** No BRender extension carries an alpha channel; a reader
  takes the alpha byte of `br_material::colour` / `br_light::colour` as opaque and carries transparency in the
  extension's own `opacity` field.
* **Unknown properties inside a `BR_*` object are skipped, not errors** - every parser falls back to
  `cgltf_skip_json()`. A file may carry properties a later version of this extension defines.
* **`extensionsUsed` is declared by this writer and not required by this reader.** Each extension written
  sets its flag (`CGLTF_EXTENSION_FLAG_BR_*`) and ends up in `extensionsUsed`; the reader only acts on
  `extensionsRequired` (see *Hard failures*).

## `BR_actors`

### What it annotates

A **node**, in place. It carries `br_actor::render_style` - the topology the renderer draws from the actor's
model: points, edges or faces, or one of the bounding-box styles.

It is not glTF's primitive `mode`. `mode` says what the indices in the file describe; `render_style` says
what BRender draws over the triangle list the file holds, and BRender draws a point or edge topology over a
triangle list (`core/v1db/render.c`, `core/v1db/modrend.c`). A file that means LINES as
geometry is refused outright (see *Hard failures*), so a line or point fixture is a triangle mesh with the
style set. In files this writer produces there is no `mode` property at all: its primitives are triangles,
which glTF takes as the default (`cgltf_write_intprop(..., "mode", ..., 4)`, `core/fmt/cgltf_write.h`).

The style is inherited down the subtree: `actorRender()` starts from the style of the most distant ancestor
with a non-default style (`core/v1db/render.c`, rule documented at `core/inc/actor.h`) and lets
any actor override it for itself and its descendants (`core/v1db/render.c`). So a node's style
applies to that node and everything below it, `BR_RSTYLE_NONE` prunes the subtree, and a node with no mesh at
all can still be the thing that changes the style of the actors under it.

### Shape

```json
{
  "name": "cube",
  "mesh": 0,
  "extensions": {
    "BR_actors": { "render_style": 3 }
  }
}
```

Abridged from `examples/rendertest/dat/scene-edges.gltf`.

### Fields

| property | JSON type | required | default (absent) | meaning |
| --- | --- | --- | --- | --- |
| `render_style` | integer | no | `0` (`BR_RSTYLE_DEFAULT`) | A `BR_RSTYLE_*` value (`core/inc/actor.h`); assigned to `br_actor::render_style`. |

| value | `BR_RSTYLE_*` | meaning |
| --- | --- | --- |
| 0 | `DEFAULT` | no style here: inherit, ending in faces |
| 1 | `NONE` | the actor and its descendants are not drawn |
| 2 | `POINTS` | vertices of each face, as points |
| 3 | `EDGES` | edges of each face |
| 4 | `FACES` | the faces |
| 5 | `BOUNDING_POINTS` | vertices of the model's bounding box |
| 6 | `BOUNDING_EDGES` | edges of the bounding box |
| 7 | `BOUNDING_FACES` | faces of the bounding box |

`BR_RSTYLE_ANTIALIASED_LINES` (8) and `BR_RSTYLE_ANTIALIASED_FACES` (9) are declared by the enum but have no
entry in the engine's dispatch table, so a reader must refuse them - see below. They are not legal values in
a file.

### Reader obligations

* **Absent extension or absent property: `BR_RSTYLE_DEFAULT`.** The parser leaves the zero-filled value
  alone (`cgltf_parse_json_brender_actor()`), and the reader writes the field only when the extension is
  present (`fill_actor()`, `core/fmt/loadgltf.c`), so it keeps the zero that `BrActorAllocate()`
  left in it - the actor's memory is zeroed on allocation and nothing sets `render_style`
  (`core/v1db/actsupt.c`).
* **Refuse `render_style` outside 0..7.** The value reaches `RenderStyleCalls[style]` with no bounds check
  (`core/v1db/render.c`) and that table has no entries above `BR_RSTYLE_BOUNDING_FACES`
  (`core/v1db/modrend.c`), so an unchecked value is a call through an out-of-range pointer. The reader
  fails the whole file with a message naming the node and the value (`check_actor_styles()`,
  `core/fmt/loadgltf.c`); it does not clamp, and it does not treat the file as loadable-minus-style.
* The value is stored in the `br_uint_8` field after the check, so the check is what keeps the field honest.

### Writer obligations

* **Write the extension only when the style is not `BR_RSTYLE_DEFAULT`** (`fill_actor_render_style_actual()`,
  `core/fmt/savegltf.c`). Writing the default would put a node extension on every actor in every file
  to say nothing.
* A writer must not emit 8 or 9. This writer refuses the save instead: it checks every actor before writing
  anything and fails with a message naming the actor and the value (`check_actor_render_styles()`,
  `core/fmt/savegltf.c`), because a file it wrote would be refused by its own reader
  (`check_actor_styles()`) and neither clamping to `BR_RSTYLE_BOUNDING_FACES` nor omitting the extension
  would mean the same thing as the value the actor carries.

### What it deliberately does not carry

The extension carries `render_style` and nothing else; the rest of `br_actor` is covered in *What no
extension carries* below.

## `BR_materials`

### What it annotates

Three places:

| place | shape | meaning |
| --- | --- | --- |
| the root | `extensions.BR_materials.materials`: an array of material objects | the material table, shared by every reference |
| a node | `extensions.BR_materials.material`: integer index | the actor's own `br_actor::material` |
| a primitive | `extensions.BR_materials.material`: integer index | the material of the faces in that primitive |

The root array is indirection because materials *are* shared: two nodes and any number of face groups may
name the same entry.

### Shape

```json
{
  "extensions": {
    "BR_materials": {
      "materials": [
        {
          "identifier": "scene-edges-material",
          "colour": [0.784313738, 0.784313738, 0.784313738],
          "map_transform": [1, 0, 0, 1, 0, 0],
          "index_base": 0,
          "index_range": 255
        }
      ]
    }
  },
  "nodes": [
    { "mesh": 0, "extensions": { "BR_materials": { "material": 0 } } }
  ]
}
```

Abridged from `examples/rendertest/dat/scene-edges.gltf`: every property not listed there is equal to its
default and was therefore omitted.

### Fields

| property | JSON type | default (absent) | `br_material` field |
| --- | --- | --- | --- |
| `identifier` | string | `NULL` | `identifier` |
| `colour` | number[3], 0..1 | `[1, 1, 1]` | `colour`, RGB; the alpha byte is written as 255 |
| `opacity` | number, 0..1 | `1` | `opacity` |
| `ka` | number | `0.1` | `ka` |
| `kd` | number | `0.7` | `kd` |
| `ks` | number | `0` | `ks` |
| `power` | number | `20` | `power` |
| `flags` | integer | `1` (`BR_MATF_LIGHT`) | `flags`, verbatim |
| `mode` | integer | `4` (`BR_MATM_DEPTH_TEST_LE`) | `mode`, verbatim |
| `map_transform` | number[6] | `[1, 0, 0, 1, 0, 0]` | `map_transform`, row-major `[m00 m01 m10 m11 m20 m21]` |
| `index_base` | integer | `10` | `index_base` |
| `index_range` | integer | `31` | `index_range` |
| `colour_map` | integer, index into `images` | absent | `colour_map` |
| `screendoor` | integer, index into `images` | absent | `screendoor` |
| `index_shade` | integer, index into `images` | absent | `index_shade` |
| `index_blend` | integer, index into `images` | absent | `index_blend` |
| `index_fog` | integer, index into `images` | absent | `index_fog` |
| `fog_min` | number | `0` | `fog_min` |
| `fog_max` | number | `0` | `fog_max` |
| `fog_colour` | number[3], 0..1 | `[0, 0, 0]` | `fog_colour`, RGB; alpha 255 |
| `subdivide_tolerance` | integer | `0` | `subdivide_tolerance` |
| `depth_bias` | number | `0` | `depth_bias` |

The defaults are the ones `BrMaterialAllocate()` leaves in a material - it copies
`SetupDefaultMaterial()` (`core/v1db/def_mat.c`), which sets `ka = 0.10`, `kd = 0.70`, `ks = 0`, `power = 20`,
`flags = BR_MATF_LIGHT`, `mode = BR_MATM_DEPTH_TEST_LE | BR_MATM_BLEND_MODE_STANDARD | wrap in both axes`, an
identity `map_transform`, `index_base = 10` and `index_range = 31`, and leaves the rest of the struct zeroed.
A material object in a file is therefore the exact state of the material, and an absent property is not a
loss.

`flags` and `mode` are the raw BRender words, not a mapping from anything in glTF - that is the point of the
extension: a material loaded through it keeps the flags and mode it was written with, while a material loaded
through glTF's own `materials` array gets its flags and mode invented by the conversion
(`fill_material()`, `core/fmt/loadgltf.c`).

### Reader obligations

* **The root `BR_materials` array replaces the material table, it does not supplement it.** If
  `extensions.BR_materials.materials` is present and non-empty, `results->nmaterials` is its length, every
  entry is built with `fill_br_material()`, and glTF's own `materials` array is not converted at all
  (`core/fmt/loadgltf.c`). A file that carries both arrays is read as a
  `BR_materials` file.
* **Absent, the material table comes from glTF's `materials` array** through `fill_material()`, and the
  `BR_materials` references are not looked for. That is the path for a file from any other tool.
* **A `material` reference that goes through glTF's own property is read as an index into the same table.**
  When the root array is present, `primitive.material` (not just `primitive.extensions.BR_materials.material`)
  indexes the `BR_materials`-derived table (`create_model()`, `core/fmt/loadgltf.c`). That works in
  files this writer produces only because it keeps `materials` and `BR_materials` parallel and of equal
  length, entry for entry; an index the BR table does not have is refused rather than read past it
  (`check_material_references()`, `core/fmt/loadgltf.c`).
* **Node references are the actor's material; primitive references are the faces' material.** A primitive
  that carries one sets it on every face in that group, and a group's material takes precedence over the
  actor's when the renderer draws it.
* **The index is taken as given, but it is range-checked.** A reference outside the table is a file error
  rather than a value to clamp, because the alternative is reading whatever follows the table.

### Writer obligations

* Write the root array whenever there is a material to write, and keep it index-parallel with glTF's
  `materials` array: `savegltf.c` writes both arrays from the same hash map enumeration
  (`data->materials_count` and `data->brender_materials_count`), so entry *n* of one describes the same
  material as entry *n* of the other.
* Write the node reference for an actor's own material (`build_node_links()`), and the primitive reference
  only for a face group that has a material of its own (`fill_mesh_materials()`, `core/fmt/savegltf.c`).
  A group whose faces carry no material must be written with no reference at all: naming one would put that
  material on the whole model on the way back in.
* Textures are referenced as `images` indices, and the `brender=` marker on those images is what makes a
  lookup table survive - see the marker section below.

### What it deliberately does not carry

* `br_material::extra_surf` and `br_material::extra_prim` (the token/value lists). Both are marked `TODO` in
  `core/fmt/cgltf_brender.h`, have no property, and are not read.
* `br_material::stored` (renderer-private prepared state) and `br_material::user` (the application's).
* The alpha byte of `colour` and `fog_colour`: both are RGB triples, with opacity carried once, in `opacity`.
* **A field that is written is not necessarily a field that is read straightforwardly: every field in the
  table above is both read and written, but `index_base`, `index_range`, `flags` and `mode` are stored in
  narrower `br_material` fields (`br_uint_8`, `br_uint_8`, `br_uint_32`, `br_uint_16`) than the JSON integer,
  and the assignment truncates rather than refusing.** Only `BR_actors.render_style` is range-checked.

## `BR_lights`

### What it annotates

Two places:

| place | shape | meaning |
| --- | --- | --- |
| the root | `extensions.BR_lights.lights`: an array of light objects | the light table |
| a node | `extensions.BR_lights.light`: integer index | this node is a light actor with that light's state |

A node with a `BR_lights` reference *is* the light: this reader decides an actor's type from the node's
`brender_light`, camera and mesh, and a node that references a `BR_lights` entry becomes `BR_ACTOR_LIGHT`
with a `br_light` built from the entry (`create_empty_actor()`, `core/fmt/loadgltf.c`). The light is
also enabled on load (`BrLightEnable()`), as the 3ds importer does.

### Fields

| property | JSON type | default (absent) | `br_light` field |
| --- | --- | --- | --- |
| `identifier` | string | `NULL` | `identifier` |
| `type` | string: `"point"`, `"direct"`, `"spot"`, `"ambient"` | `"direct"` | `type`, `BR_LIGHT_TYPE` bits |
| `view_space` | boolean | `false` | `type \|= BR_LIGHT_VIEW` |
| `linear_falloff` | boolean | `false` | `type \|= BR_LIGHT_LINEAR_FALLOFF` |
| `colour` | number[3], 0..1 | `[1, 1, 1]` | `colour`, RGB; alpha 255 |
| `attenuation_c` | number | `1` | `attenuation_c` |
| `attenuation_l` | number | `0` | `attenuation_l` |
| `attenuation_q` | number | `0` | `attenuation_q` |
| `cone_outer` | number, turns | `15/360` | `cone_outer` |
| `cone_inner` | number, turns | `10/360` | `cone_inner` |
| `radius_outer` | number | `0` | `radius_outer` |
| `radius_inner` | number | `0` | `radius_inner` |
| `volume` | object: `falloff_distance`, `regions` | absent | `volume` |

The defaults are those of a light actor from `BrActorAllocate()` - a directional light, white, constant
attenuation 1, cones of 15 and 10 degrees (`core/v1db/actsupt.c`).

Angles are in turns, as everywhere in BRender: 1.0 is a full circle, and the defaults are written as
`BR_ANGLE_DEG(15)`/`BR_ANGLE_DEG(10)` - `15.0f / 360.0f` in the table above. `radius_outer` and
`radius_inner` are distances: sphere radii used for the linear falloff and the cutoff of a normally
attenuated light, not angles (`core/inc/light.h`).

`attenuation_c`, `attenuation_l` and `attenuation_q` are the constant, linear and quadratic terms of the
engine's attenuation, which for a point or spot light is `1 / (c + l·d + q·d²)` for distance `d`
(`drivers/softrend/lightmac.h`). No fixture in this repository sets `attenuation_l` or
`attenuation_q`; they carry their defaults.

`volume` is a cutoff volume (`br_light_volume`): a set of convex regions, each the intersection of a
number of half-spaces, together with a falloff distance. A vertex inside any region is lit in full;
beyond a region's boundary the light fades linearly to nothing over `falloff_distance`. `regions` is an
array of regions and each region an array of planes, each plane four numbers `[x, y, z, w]` - a
`br_vector4`. The planes are in the **light actor's local space**, exactly as `br_light_volume` holds
them; the core transforms them to view space when the light is set up (`core/inc/light.h`,
`core/inc/brvector.h`, `core/v1db/enables.c`).

### Reader obligations

* **Absent, the node is not a light.** No `brender_light` reference means `create_empty_actor()` cannot make
  a `BR_ACTOR_LIGHT`, so the node is not a light actor and nothing about lighting changes. A file that
  carries its lights only in `KHR_lights_punctual` loads with them dropped and logged as an ignored
  extension, because this reader never looks at `cgltf_node::light`; that is a stated limitation, not a
  defect, and *Hard failures* says why.
* **`type` is not a closed set.** An unrecognised string leaves the default `"direct"`, and an absent
  `type` reads as `"direct"`; `view_space` and `linear_falloff` are separate booleans ORed into
  `br_light::type` (`fill_light()`, `core/fmt/loadgltf.c`).
* **The light table is the `BR_lights` array.** `results->nlights` and `results->lights` are sized and
  filled from `data->brender_lights_count` (`core/fmt/loadgltf.c`), one `br_light` per entry, and each
  light actor's own `br_light` is filled from the entry its node references. The KHR light array is not
  consulted, so a file whose two arrays differ in length is still readable: a `BR_lights` entry no node
  references simply has no actor behind it, and a file with `BR_lights` and no `KHR_lights_punctual` at
  all loads.
* `cone_outer`/`cone_inner` are read for any light and `radius_outer`/`radius_inner` for any light, although
  the writer only writes them in the circumstances described next.
* **The volume is only read when present, and carries no state when absent.** `falloff_distance`
  defaults to `0` and `regions` to empty, so a light with no `volume` property reads back with an empty
  `br_light_volume` - indistinguishable from one whose `volume` had no regions. Each plane is four
  numbers and each region an array of planes (`cgltf_parse_json_brender_light_volume()`,
  `core/fmt/cgltf_brender_impl.h`); the region and plane arrays are allocated with the rest of the light
  table and freed with it.

### Writer obligations

* Write the root array when there are lights, and keep it index-parallel with `KHR_lights_punctual`: both are
  counted and allocated together and both indices advance together (`gather_models_and_materials()`
  and `build_node_links()`, `core/fmt/savegltf.c`).
* **Also write the lossy `KHR_lights_punctual` projection, including a dummy entry per ambient light.** A
  `BR_ACTOR_LIGHT` writes both `node.light` and `node.brender_light` (`fill_actor_types_actual()`,
  `core/fmt/savegltf.c`), and for an ambient light - which KHR cannot express - a placeholder point
  light with intensity 0 is written into the array while the node's KHR reference is dropped. The placeholder
  is what keeps the two indices equal for the lights after it; a glTF-only reader sees an unreferenced dummy.
  The projection is written for consumers other than this reader and is never read back: `KHR_lights_punctual`
  is named in `extensionsUsed` only, so a file this writer produces loads whatever the loader does with the
  extension.
* **`cone_outer` and `cone_inner` are written only for a spot light, and only when they differ from their
  defaults; `radius_outer` and `radius_inner` only for a light with `linear_falloff`**
  (`cgltf_write_brender_light()`, `core/fmt/cgltf_write_brender.h`). Cones on a light that is not a spot
  and radii on a light without linear falloff are therefore not written back, even though the reader would
  accept them.
* `type` is omitted when the type word is not one of the four (`cgltf_brender_light_type_invalid`), which
  reads back as `"direct"`.
* **The volume is written only when there is at least one region**, and `falloff_distance` within it only
  when non-zero (`cgltf_write_brender_light()`, `core/fmt/cgltf_write_brender.h`). A light whose volume
  is empty is written with no `volume` property at all, which reads back as the same empty volume.
* The KHR projection is lossy by construction: it derives the KHR `range` from the attenuation terms with
  `atten_to_range()` (`core/fmt/savegltf.c`), so a light that never attenuates with distance - the
  default `c = 1, l = 0, q = 0` - is written with the largest float as its range (`3.40282347e+38` in six
  fixtures). This reader never reads that field back.

### What it deliberately does not carry

* `br_light::user`. It is application state rather than engine state, and the reader and writer leave it
  alone.
* The RGB alpha byte, as for `BR_materials`.

## The `brender=` image URI marker

This one is **not a glTF extension**: there is no `extensions` object and no `extensionsUsed` entry. It is a
parameter in the media type of a base64 data URI, so a reader meets it wherever it meets an image. It is
documented here because it exists for `BR_materials`: a material's lookup tables are pixelmaps the
rasterisers index directly, and glTF has no way to say "these bytes are not colour".

```
"images": [
  { "name": "scene-tex-rgb565-map",
    "uri": "data:image/png;brender=rgb565;base64,iVBORw0KGgoAAAANSUhEUgA..." }
]
```

| marker | PNG channels | pixelmap type on load | samples |
| --- | --- | --- | --- |
| `brender=index8` | 1 | `BR_PMT_INDEX_8` | one byte per sample, an index |
| `brender=rgb555` | 2 | `BR_PMT_RGB_555` | one output word per sample |
| `brender=rgb565` | 2 | `BR_PMT_RGB_565` | one output word per sample |
| `brender=rgb888` | 3 | `BR_PMT_RGB_888` | one output pixel per sample |

The marker names the type rather than leaving it to be inferred, because `BR_PMT_RGB_555` and
`BR_PMT_RGB_565` are both two bytes per sample: one marker for both would put a 565 table into a 555
output's material. A shade table's type must equal the output's.

* **Writer**: a pixelmap with **no palette** (`pm->map == NULL`) and one of those four types is written as a
  palette-less PNG whose channels *are* the pixelmap's samples, marked with its type
  (`fill_pixelmap()`, `core/fmt/savegltf.c`). The trigger is the pixelmap, not its slot, so a
  palette-less colour map is carried this way too - which is the only faithful thing to do, since there is no
  palette to expand it through. The marker is only ever put on a base64 data URI, and the PNG codec is
  lossless, so the samples come back byte for byte.
* **Reader**: the marker is recognised on a base64 data URI only, and the image is rebuilt as the marked
  pixelmap type with the samples in their own channels and **no palette**, whatever slot it is referenced
  from (`load_table_type()` and `load_pixelmap()`, `core/fmt/loadgltf.c`). It is not converted to
  `br_gltf_options::pm_type`, unlike an ordinary image. The PNG is decoded at its own channel count and the
  marker's count is checked against it: an image whose channels disagree with its marker is refused rather
  than converted, because stbi converts silently and the conversion would rebuild a table of the wrong bytes
  (`image_marker_mismatch()`, `core/fmt/loadgltf.c`).
* **Absent marker, or a pixelmap that has a palette**: the PNG is colour. The reader decodes it and converts
  it to `br_gltf_options::pm_type`; the writer expands it through its palette and writes an ordinary PNG. A
  reader that does not know the marker decodes the marked PNG as an image and gets the samples interpreted as
  grey/colour - wrong pixels, but no error, which is the reason the marker exists rather than a convention.
* The URI must be a base64 data URI: the reader only decodes a URI that carries `;base64,` before the comma
  (`unbuild_data_url()`, `core/fmt/loadgltf.c`), and otherwise treats the whole URI as a path relative
  to `br_gltf_options::base_path`.
* **A missing image is not a failure; a broken one is.** An image the loader could not obtain - a URI naming
  a file that is not there, or one it cannot read - is left as a NULL entry in `results->pixelmaps`, and the
  material that referenced it keeps no map, so the scene loads untextured (see *Hard failures* for the
  images that refuse the load instead).
* 41 corpus fixtures carry at least one marked image - 37 an index8 one, 7 rgb555, 7 rgb565 and 4 rgb888 -
  referenced from `colour_map`, `index_shade`, `index_blend` and `index_fog`.

## Hard failures

This reader refuses, rather than converts, eleven kinds of file, and a refused file is refused whole, with
nothing handed back (`BrFmtGLTFActorLoadMany()`, `core/fmt/loadgltf.c`). Eight of them are pure and run
before the buffers are loaded; the other three read the buffers - a buffer file is measured as it is loaded,
and the index range check and the image decode run once the buffers are in.

| what | behaviour |
| --- | --- |
| an `extensionsRequired` entry other than `BR_actors`, `BR_lights`, `BR_materials`, `KHR_texture_transform` | the file is refused, which is what glTF 2.0 requires for an extension the loader does not implement. This includes `KHR_lights_punctual`: the writer emits a lossy projection of the lights into it, but it names the extension only in `extensionsUsed`, never in `extensionsRequired`, so its own files load; a file that *requires* it is refused rather than read with its lights dropped. |
| a primitive whose glTF mode is not TRIANGLES, TRIANGLE_STRIP or TRIANGLE_FAN | the file is refused, with the mesh, the primitive index and the mode named. This is the state `render_style` cannot carry: BRender has no representation for stored point or line geometry, so a file that really means mode 0..3 has no faithful reading. |
| a primitive with no attributes, or with no POSITION among them | the file is refused, naming the mesh and the primitive. The model's vertex count is taken from the primitive's first attribute and its vertices' positions from POSITION, and glTF requires both; without them this loader reads past the attributes array, or builds a model whose every vertex is left at the origin. |
| a primitive whose attributes do not all have the same element count | the file is refused, naming the mesh, the primitive, the attribute and both counts. Every attribute is read once per vertex, up to the first attribute's count, so a shorter one is read past its own end. |
| a primitive whose index or vertex count does not fit its mode | the file is refused, naming the mesh, the primitive and the count. A TRIANGLES primitive is sized for count/3 faces and filled three indices at a time, so a count that is not a positive multiple of three is filled with one face more than was allocated; a strip or a fan with fewer than three derives a face count from a subtraction that underflows. |
| an index that names a vertex the primitive does not have | the file is refused, naming the mesh, the primitive, the index and the number of vertices. Such a model is one BRender's own preparation refuses ("face references invalid vertex"), which would abort the caller from inside the renderer instead of failing to load. |
| a `BR_actors.render_style` outside `BR_RSTYLE_DEFAULT..BR_RSTYLE_BOUNDING_FACES` | the file is refused, naming the node and the value. |
| a `primitive.material` index past the `BR_materials` root array, in a file that carries one | the file is refused, naming the mesh, the primitive, the index and the table length. Such an index is read as an index into the `BR_materials`-derived table, so it would otherwise be an unchecked read past the table. |
| a buffer view, or an accessor under one, that reaches past the end of its buffer | the file is refused, naming the buffer view and its offset, size and buffer, or the accessor and the range it reaches. `cgltf_buffer_view_data()` returns `buffer->data + view->offset` and does no bounds check, and every read this loader makes through a view starts there: the image path hands `view->size` bytes to `stbi_load_from_memory()`, and the geometry, index and animation paths read accessors through it. A view whose offset or offset-plus-size runs past its buffer, or an accessor whose offset and stride do, would read whatever follows the allocation. |
| an external buffer file shorter than the `byteLength` it declares | the file is refused, naming the file and both lengths. `cgltf_load_buffer_file()` reads the file and never writes the length it read back to `data->buffers[i].size`, so `buffer->size` stays the declared `byteLength` while the allocation is only what was read; a view inside the declared range is therefore past the end of the allocation, and the view and accessor checks above, which compare against the declared size, cannot see it. `cgltf_load_brfile()` measures the file as it loads it, the one place the loaded length is known. |
| an image that is present but could not be decoded, or a marked lookup table whose PNG does not carry the channels its marker names | the file is refused, naming the image's index and URI, or its index, name and marker. A malformed base64 data URI, a zero-byte or truncated file, and a buffer view with no data are all the image being there and broken, which is a bug in the asset; a marker whose channel count disagrees with the PNG is the same, because stbi would convert between the two and silently rebuild the wrong table instead. This is deliberately not the same as a *missing* image - one the loader could not obtain, such as a URI naming a file that is not there or that it cannot read - which is left as a NULL entry because an image is optional (see the marker section above). |

The four primitive checks are `check_primitive_attributes()`, `check_primitive_attribute_counts()`,
`check_primitive_counts()` and `check_primitive_index_ranges()` (`core/fmt/loadgltf.c`), and every one of
them is an invariant glTF states that the parser does not enforce: this reader does not call
`cgltf_validate()`, so a file is held to the grammar only as far as the fields it reads are concerned.

The buffer view and the accessor under it are checked the same way, by `check_buffer_views()` and
`check_accessor_ranges()` (`core/fmt/loadgltf.c`). Both are invariants
glTF states and cgltf_validate() checks, which this reader does not call: a view must lie within its buffer,
and an accessor's elements must lie within their view. A buffer file's length is checked against the
`byteLength` it declares in `cgltf_load_brfile()` (`core/fmt/loadgltf.c`), the file callback, because
the loaded length is not carried anywhere else and the declared size is what every other check compares
against.

An `extensionsUsed` entry this loader does not implement is instead logged and ignored, because a file that
merely uses an extension may be read without it. For `KHR_lights_punctual` that means a file carrying its
lights only there loads with them dropped; this is a stated limitation, not a round-trip defect. The writer's
own files carry `BR_lights` as well, and that is the channel this reader reads, while the KHR projection
exists so that a consumer which is not this reader can see the lights at all. Reading the projection back
would need a KHR-to-`br_light` mapping that the writer's own reverse mapping already states does not exist
(`fill_light()`, `core/fmt/savegltf.c`).

## What no extension carries

State that survives a `.gltf` round trip only by being derived from something else, or not at all. None of
this is a defect of the extensions as specified above; it is what a file can hold.

| state | what happens |
| --- | --- |
| `br_actor::type` for `BOUNDS`, `BOUNDS_CORRECT`, `CLIP_PLANE`, `HORIZON_PLANE` | back as `BR_ACTOR_NONE`. `create_empty_actor()` can only produce `NONE`, `MODEL`, `LIGHT` or `CAMERA`, from the node's mesh, camera and light. The two bounds types are what makes `actorRender()` *truncate* a subtree (`core/v1db/render.c`), so a culled tree no longer culls; the clip and horizon planes are enabled through a different path (`core/v1db/enables.c`) and are simply gone. A `BR_ACTOR_MODEL` whose model is `NULL` also has no mesh to write and comes back as `NONE`. |
| `br_actor::t`'s representation | the transform survives, its representation does not. `fill_transform()` writes a 4x4 matrix for `MATRIX34`, `MATRIX34_LP` and `LOOK_UP`, a rotation for `QUAT` and `EULER`, a translation for `TRANSLATION` and nothing for `IDENTITY`; `read_actor_matrix()` reads back only `MATRIX34`, `TRANSLATION`, `QUAT` or `IDENTITY`. So `EULER` returns as `QUAT`, `LOOK_UP` and `MATRIX34_LP` as `MATRIX34`, and an identity `MATRIX34` as `IDENTITY` - equal transforms, so no pixel moves, and the value a caller reads back out of `br_actor::t` is of a different kind. This is a property of glTF's node transform, which is a matrix or a TRS triple and says nothing about which BRender form produced it. |
| `br_actor::render_data`, `br_actor::user` | pointers into the running process; no field. |
| `br_model::pivot`, `::flags`, `::crease_angle`, per-face `smoothing` and `flags`, per-face colours | nothing carries these. There is no mesh-level extension; only `BR_MODF_CUSTOM_NORMALS` is re-derived, from the presence of a NORMAL accessor. |
| `br_material::extra_surf`, `extra_prim`, `stored`, `user`; `br_light::user` | nothing carries these. |
