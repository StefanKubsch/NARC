#version 450 core
in vec2 Texcoord;
out vec4 outColor;
uniform float Opacity;
uniform sampler2D Texture;
void main()
{
    outColor = texture(Texture, Texcoord);
	outColor.a *= Opacity;
}