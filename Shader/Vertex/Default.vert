#version 450 core
in vec2 position;
in vec2 texcoord;
out vec2 Texcoord;
uniform mat4 MVP;
void main()
{
    Texcoord = texcoord;
    gl_Position = MVP * vec4(position, 0.0f, 1.0f);
}