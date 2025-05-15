#version 150

layout(std140) uniform RectData {
    mat4  mvp;
    vec4  src_rect;
    vec4  dst_rect;
    float vertical_flip;
    float indexed;
} rect_data;

in vec2 uv;

uniform sampler2D uSampler;
uniform usampler2D uIndexTex;

out vec4 main_colour;

void main()
{
    vec2 mappedUV = vec2(uv.x, abs(rect_data.vertical_flip - uv.y));
    vec4 texColour;

    if(rect_data.indexed > 0) {
        int index = int(texture(uIndexTex, mappedUV).r);
        texColour = texelFetch(uSampler, ivec2(0, index), 0);
    } else {
        ivec2 size = textureSize(uSampler, 0);
        texColour = texelFetch(uSampler, ivec2(mappedUV.x * size.x, mappedUV.y * size.y), 0);
    }

    vec3 colour = vec3(1);

    main_colour = texColour * vec4(colour.rgb, 1.0);
}
