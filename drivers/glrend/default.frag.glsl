#version 150

in vec3 colour;
in vec2 uv;
uniform sampler2D uSampler;
uniform usampler2D uIndexTex;
uniform float uVerticalFlip;
uniform float uIndexed;

out vec4 mainColour;

void main()
{
    vec2 mappedUV = vec2(uv.x, abs(uVerticalFlip - uv.y));
    vec4 texColour;

    if(uIndexed > 0) {
        int index = int(texture(uIndexTex, mappedUV).r);
        texColour = texelFetch(uSampler, ivec2(0, index), 0);
    } else {
        texColour = texture(uSampler, mappedUV);
    }

    mainColour = texColour * vec4(colour.rgb, 1.0);
}
