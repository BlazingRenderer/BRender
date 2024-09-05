#version 150
in vec3 aPosition;
in vec3 aColour;
in vec2 aUV;
out vec3 colour;
out vec2 uv;

uniform mat4 uMVP;

void main()
{
    gl_Position = uMVP * vec4(aPosition, 1.0);
    colour = aColour;
    uv = aUV;
}
