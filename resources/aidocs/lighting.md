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

**Note**: removing the upper clamp was attempted but ineffective for glrend1x — `glColor4f` also clamps to [0,1] per the GL 1.2 spec.

## Historical: glrend commit 1a390c47

Split the old combined `diffuse = clamp(lightColour, 0, 1)` into separate `outA`, `outD`, `outS` accumulators with no intermediate clamp. This deferred clamping to the final fragment output, fixing an edge-matching issue where textured and adjacent untextured polygon colours diverged due to the clamp happening before `surfaceColour * texColour` modulation.

## Active Edge-Matching Investigation

**Symptom**: In Croc scenes, textured polygon edges should match the colour of adjacent untextured polygons (same material). glrend matches; softrend/glrend1x don't.

**Ruled out**: specular (Croc `ks=0`), specular-texture interaction, removing `CALCULATE_COMPONENT` upper clamp (glColor re-clamps).

**Under investigation**: `glColor4f` clamp to [0,1] in GL 1.2 — vertex colours exceeding 1.0 get clamped before texture modulation. glrend avoids this via unclamped `vec3` varyings.
