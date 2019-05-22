/*
****************************************************
*                                                  *
* lwmf_openglshader - lightweight media framework  *
*                                                  *
* (C) 2019 - present by Stefan Kubsch              *
*                                                  *
****************************************************
*/

#pragma once

#include <cstdint>
#include <GL/gl.h>

namespace lwmf
{


	void InitShader(TextureStruct& Texture);
	void ClearBuffer();
	void RenderTexture(const TextureStruct& Texture);

	//
	// Variables and constants
	//

	inline GLuint TextureID{};

	//
	// Functions
	//

	inline void InitShader(TextureStruct& Texture)
	{
		constexpr GLfloat Vertices[]
		{
			//  Position		Texcoords
				-1.0F,  1.0F,	0.0F, 0.0F,		// Top-left
				 1.0F,  1.0F,	1.0F, 0.0F,		// Top-right
				 1.0F, -1.0F,	1.0F, 1.0F,		// Bottom-right
				-1.0F, -1.0F,	0.0F, 1.0F		// Bottom-left
		};

		constexpr GLint Elements[]
		{
			0, 1, 2,
			2, 3, 0
		};

		GLuint ElementBufferObject{};
		GLuint VertexBufferObject{};
		GLuint VertexArrayObject{};

		const GLchar* VertexShaderString[]{
			"#version 450 core\n"
			"layout (location = 0) in vec2 position;\n"
			"layout (location = 1) in vec2 texcoord;\n"
			"out vec2 Texcoord;\n"
			"void main()\n"
			"{\n"
			"Texcoord = texcoord;\n"
			"gl_Position = vec4(position, 0.0f, 1.0f);\n"
			"}"
		};

		const GLchar* FragmentShaderString[]{
			"#version 450 core\n"
			"in vec2 Texcoord;\n"
			"out vec4 outColor;\n"
			"uniform sampler2D Texture;\n"
			"void main()\n"
			"{\n"
			"outColor = texture2D(Texture, Texcoord);\n"
			"}"
		};

		// Create buffers
		glGenBuffers(1, &VertexBufferObject);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

		glGenVertexArrays(1, &VertexArrayObject);
		glBindVertexArray(VertexArrayObject);

		glGenBuffers(1, &ElementBufferObject);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBufferObject);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Elements), Elements, GL_STATIC_DRAW);

		// Create and compile the vertex shader
		const GLint VertexShader{ glCreateShader(GL_VERTEX_SHADER) };
		glShaderSource(VertexShader, 1, VertexShaderString, nullptr);
		glCompileShader(VertexShader);

		// Create and compile the fragment shader
		const GLint FragmentShader{ glCreateShader(GL_FRAGMENT_SHADER) };
		glShaderSource(FragmentShader, 1, FragmentShaderString, nullptr);
		glCompileShader(FragmentShader);

		// Link the vertex and fragment shader std::int_fast32_to a shader program
		const GLint ShaderProgram{ glCreateProgram() };
		glAttachShader(ShaderProgram, VertexShader);
		glAttachShader(ShaderProgram, FragmentShader);
		glBindFragDataLocation(ShaderProgram, 0, "outColor");
		glLinkProgram(ShaderProgram);
		glUseProgram(ShaderProgram);

		// Specify the layout of the vertex data
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) << 2, nullptr);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) << 2, reinterpret_cast<GLvoid*>(sizeof(GLfloat) << 1)); //-V566

		// Detach & delete shader
		glDetachShader(ShaderProgram, FragmentShader);
		glDetachShader(ShaderProgram, VertexShader);
		glDeleteShader(FragmentShader);
		glDeleteShader(VertexShader);

		// Bind a texture
		glGenTextures(1, &TextureID);
		glBindTexture(GL_TEXTURE_2D, TextureID);

		if (FullscreenFlag == 1)
		{
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, Texture.Width, Texture.Height);
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glDisable(GL_BLEND);
	}

	inline void ClearBuffer()
	{
		glColor4f(0.0F, 0.0F, 0.0F, 0.0F);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	inline void RenderTexture(const TextureStruct& Texture)
	{
		switch (FullscreenFlag)
		{
		case 1:
		{
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Texture.Width, Texture.Height, GL_RGBA, GL_UNSIGNED_BYTE, Texture.Pixels.data());
			break;
		}
		case 0:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Texture.Width, Texture.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Texture.Pixels.data());
			break;
		}
		default: {};
		}

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	}

	
} // namespace lwmf