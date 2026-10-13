# Lighting

## Lighting Equation

```
output = prelit + surface_colour * (ambient + diffuse) + specular
```

- `ambient = ka * ambient_colour` (or just `ka` if no ambient lights)
- `diffuse = sum( kd * max(N·L, 0) * light_colour * attenuation )` over all non-ambient lights
- `prelit` is vertex colour from model data (only when `BRT_PRELIGHTING_B` set)

## Code Locations

### Ambient
- **softrend**: `light24.c:160-171` (`SurfaceColourLit`), `setup.c:485-525` (ambient accumulation)
- **glrend**: `brender.common.glsl:243-247` (`accumulateLights`), `cache.c:58-61` (CPU-side accumulation)

### Diffuse (per-light)
- **softrend**: `light24.c` — `lightingColourDirect` (469), `lightingColourPoint` (538), `lightingColourPointAttn` (633), `lightingColourPointRadii` (851)
- **glrend**: `brender.common.glsl` — `lightingColourDirect` (129), `lightingColourPoint` (146), `lightingColourSpot` (185)
- Macros (`lightmac.h`): `ACCUMULATE_DIFFUSE_RGB` (226), `SPOT_DOT` (178), `SPOT_FALLOFF` (186)

### Specular
- **softrend**: Blinn-Phong for directional (`SPECULAR_DOT_DIRECT`), Phong for point/spot (`SPECULAR_DOT`). Power via Gems IV approximation (`lightmac.h:66-67`).
- **glrend**: Blinn-Phong for all lights. `pow(specDot, power)`.
- Croc does not use specular (`ks = 0`).

### Attenuation
- **softrend**: `lightmac.h` — `CALCULATE_ATTENUATION` (120), `CALCULATE_ATTENUATION_LINEAR` (132), `CALCULATE_ATTENUATION_RADII` (142)
- **glrend**: `brender.common.glsl` — `calculateAttenuation` (84), `calculateAttenuationRadii` (93)

### Prelighting / No Lighting
- **softrend**: `SurfaceColourLitPrelit` (`light24.c:243`), `SurfaceColourUnlit` (`light24.c:126`)
- **glrend**: `brender.frag.glsl:170-171`

### Surface Colour Application
- **softrend**: `genrend.c` — `Vertex_SurfaceComponentsGeom` (439) uses `rend.vertex_colours[v]`, `Vertex_SurfaceComponentsSurf` (379) uses `scache.colour`
- **glrend**: `v1model.c:257-270` (UBO population), `brender.frag.glsl:168` (shader selection)

## CALCULATE_COMPONENT Macro

Defined in `drivers/softrend/lightmac.h:27-37`. Final per-vertex step:

```c
#define CALCULATE_COMPONENT(c, colour, prelit, diffuse, specular)                                              \
    do {                                                                                                       \
        br_scalar total;                                                                                       \
        total = prelit + BR_MUL(colour, diffuse) + specular;                                                   \
        if(BR_SCALAR_GE_1(total))                                                                              \
            comp[(c)] = self->state.cache.comp_offsets[(c)] + self->state.cache.comp_scales[(c)] - BR_SCALAR_EPSILON; \
        else if(BR_SCALAR_LE_0(total))                                                                         \
            comp[(c)] = self->state.cache.comp_offsets[(c)];                                                   \
        else                                                                                                   \
            comp[(c)] = BR_MUL(self->state.cache.comp_scales[(c)], total) + self->state.cache.comp_offsets[(c)]; \
    } while(0)
```

Clamps per-vertex lighting to [0, 1] before the rasteriser modulates by texture. The upper clamp can cause colour mismatches when `lighting > 1.0`.

**Note**: removing the upper clamp was attempted but ineffective for glrend1x — `glColor4f` also clamps to [0,1] per the GL 1.2 spec. Neither is the cause of the Croc textured-edge mismatch; that is the unlit RGB-output block, see "Textured Polygons Are Unlit at RGB Output" below.

## Historical: glrend commit 1a390c47

Split the old combined `diffuse = clamp(lightColour, 0, 1)` into separate `outA`, `outD`, `outS` accumulators with no intermediate clamp. This deferred clamping to the final fragment output, fixing an edge-matching issue where textured and adjacent untextured polygon colours diverged due to the clamp happening before `surfaceColour * texColour` modulation.

## Textured Polygons Are Unlit at RGB Output

**Symptom**: In Croc scenes, textured polygon edges do not match the colour of adjacent untextured polygons (same material). glrend matches; softrend/glrend1x do not.

**Cause**: an RGB-output textured primitive has no lit block to select. The rasteriser's tables pair a lit shade only with an `INDEX_8` texture — the lit textured blocks are the shade-table family (`TriangleRenderPIZ2TIA_RGB_888`), whose fragment is `shade[(intensity << 8) | texel]` and whose texture must be `BR_PMT_INDEX_8`. A `BR_PMT_RGB_888` texture can only match the unlit `SHADE_NONE` block, whose `vertex_components` (`0x1f0` — `CM_SX|CM_SY|CM_SZ|CM_U|CM_V`) carry no colour or intensity bit, so `GenerateSurfaceFunctions()` (`setup.c:423`, called from `setup.c:227`) never emits a lit surface function and the texel passes straight through.

At 24bpp, rendertest's loader de-CLUTs every image to the colour buffer's type (`core/fmt/loadgltf.c:362`, `BrPixelmapCloneTyped(tmp, opts.pm_type)`), so the textures are `RGB_888` and the lit shade-table blocks can never fire. softrend's `scene-textured` mean is exactly the texture's own mean (127.34) and its lit and unlit renders are byte-identical (type 6: both `7f2255d13547467b`); glrend lights the same texture (11.19). The Croc scenes are the same mechanism: `croc-title-ice` is 102.33 (the texture) in softrend against 135.44 in glrend.

**The clamp is not the cause.** `CALCULATE_COMPONENT`'s upper clamp was the documented suspect. It is not: making the `total >= 1` arm unreachable (`lightmac.h`) moves nothing — `croc-title-ice` 102.33 -> 102.33, `scene-textured` 127.34 -> 127.34, `scene-smooth` 20.28 -> 20.28, byte-identical. The clamp never fires on these scenes, and `glColor4f` re-clamping on the GL 1.2 path is likewise not the mechanism (`glrend1x` shares softrend's block selection). It can still affect other content, but it is not the cause here.

**Expected, not under investigation.** This is pentprim's block set faithfully ported: pentprim's checksum for `scene-textured` at 24bpp is the same `7f2255d13547467b`. glrend lights textured `RGB_888` polygons; the software rasterisers cannot, because no such block exists in the table. Adding one is a reference decision, not a bug fix — the `x87` reference is pentprim's, and pentprim has no lit `RGB_888`-texture arm. See issue #30 for the block-level analysis and the two levers; the behaviour is kept as-is deliberately.
