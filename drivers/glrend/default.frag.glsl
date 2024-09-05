#version 150

in vec3 colour;
in vec2 uv;
uniform sampler2D uSampler;
uniform float uVerticalFlip;

out vec4 mainColour;

void main()
{
    mainColour = texture(uSampler, vec2(uv.x, abs(uVerticalFlip - uv.y))) * vec4(colour.rgb, 1.0);
}
