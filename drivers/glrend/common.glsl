vec4 bilinearFilter(sampler2D sampler, vec2 uv)
{
    // Get the size of the texture (width and height in texels)
    vec2 textureSize = textureSize(sampler, 0);

    // Convert UVs from [0,1] range to texture coordinate space
    vec2 texCoord = uv * textureSize;

    // Get the integer and fractional parts of the texture coordinates
    vec2 texCoordFloor = floor(texCoord);
    vec2 texCoordFrac = fract(texCoord);

    // Get the four surrounding texels
    vec2 texel00 = (texCoordFloor + vec2(0.0, 0.0)) / textureSize; // Top-left
    vec2 texel10 = (texCoordFloor + vec2(1.0, 0.0)) / textureSize; // Top-right
    vec2 texel01 = (texCoordFloor + vec2(0.0, 1.0)) / textureSize; // Bottom-left
    vec2 texel11 = (texCoordFloor + vec2(1.0, 1.0)) / textureSize; // Bottom-right

    // Fetch the texel colors
    vec4 color00 = texture(sampler, texel00); // Top-left
    vec4 color10 = texture(sampler, texel10); // Top-right
    vec4 color01 = texture(sampler, texel01); // Bottom-left
    vec4 color11 = texture(sampler, texel11); // Bottom-right

    // Bilinear interpolation
    vec4 topMix = mix(color00, color10, texCoordFrac.x);  // Interpolate horizontally on the top row
    vec4 bottomMix = mix(color01, color11, texCoordFrac.x);  // Interpolate horizontally on the bottom row
    vec4 finalColor = mix(topMix, bottomMix, texCoordFrac.y); // Interpolate vertically

    return finalColor; // Return the bilinearly filtered color
}

vec4 texturei(usampler2D index, sampler2D clut, vec2 uv)
{
    int i = int(texture(index, uv).r);
    return texelFetch(clut, ivec2(0, i), 0);
}

vec4 bilinearFilter(usampler2D index, sampler2D clut, vec2 uv)
{
    // Get the size of the texture (width and height in texels)
    vec2 textureSize = vec2(textureSize(index, 0));

    // Convert UVs from [0,1] range to texture coordinate space
    vec2 texCoord = uv * textureSize;

    // Get the integer and fractional parts of the texture coordinates
    vec2 texCoordFloor = floor(texCoord);
    vec2 texCoordFrac = fract(texCoord);

    // Get the four surrounding texels
    vec2 texel00 = (texCoordFloor + vec2(0.0, 0.0)) / textureSize; // Top-left
    vec2 texel10 = (texCoordFloor + vec2(1.0, 0.0)) / textureSize; // Top-right
    vec2 texel01 = (texCoordFloor + vec2(0.0, 1.0)) / textureSize; // Bottom-left
    vec2 texel11 = (texCoordFloor + vec2(1.0, 1.0)) / textureSize; // Bottom-right

    // Fetch the texel colors
    vec4 color00 = texturei(index, clut, texel00); // Top-left
    vec4 color10 = texturei(index, clut, texel10); // Top-right
    vec4 color01 = texturei(index, clut, texel01); // Bottom-left
    vec4 color11 = texturei(index, clut, texel11); // Bottom-right

    // Bilinear interpolation
    vec4 topMix = mix(color00, color10, texCoordFrac.x);  // Interpolate horizontally on the top row
    vec4 bottomMix = mix(color01, color11, texCoordFrac.x);  // Interpolate horizontally on the bottom row
    vec4 finalColor = mix(topMix, bottomMix, texCoordFrac.y); // Interpolate vertically

    return finalColor; // Return the bilinearly filtered color
}
