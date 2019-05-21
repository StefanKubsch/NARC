#version 450 core
in vec2 Texcoord;
out vec4 outColor;
uniform sampler2D Texture;
void main()
{
    outColor = texture2D(Texture, Texcoord);
}