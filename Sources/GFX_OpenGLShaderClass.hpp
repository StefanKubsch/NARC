/*
******************************************
*                                        *
* GFX_ShaderClass.hpp                    *
*                                        *
* (c) 2017, 2018, 2019 Stefan Kubsch     *
******************************************
*/

#pragma once

#define FMT_HEADER_ONLY
#define glCheckError() CheckError(__LINE__)

#include <cstdint>
#include <string>
#include <fstream>
#include <limits>
#include <sstream>
#include "fmt/format.h"

#include "Tools_Console.hpp"
#include "Tools_ErrorHandling.hpp"

class GFX_OpenGLShaderClass final
{
public:
	void LoadShader(const std::string& ShaderName);
	void LoadTextureInGPU(const lwmf::TextureStruct& TextureData, GLuint* Texture);
	void LoadSurfaceInGPU(const SDL_Surface* Surface, GLuint* Texture);
	void RenderTexture(const GLuint* Texture, std::int_fast32_t PosX, std::int_fast32_t PosY, std::int_fast32_t Width, std::int_fast32_t Height);
	void LoadStaticTextureInGPU(const lwmf::TextureStruct& TextureData, GLuint* Texture, std::int_fast32_t PosX, std::int_fast32_t PosY, std::int_fast32_t Width, std::int_fast32_t Height);
	void RenderStaticTexture(const GLuint* Texture);
	void PreparePixelBufferTexture(std::int_fast32_t PosX, std::int_fast32_t PosY, std::int_fast32_t Width, std::int_fast32_t Height);
	void RenderPixelBufferTexture();

private:
	enum class Components : std::int_fast32_t
	{
		Shader,
		Program
	};

	void Ortho2D(GLfloat* Matrix, GLfloat Left, GLfloat Right, GLfloat Bottom, GLfloat Top);
	void UpdateVertices(std::int_fast32_t PosX, std::int_fast32_t PosY, std::int_fast32_t Width, std::int_fast32_t Height);
	const char* LoadShaderSource(const std::string& FileName);
	void CheckError(std::int_fast32_t Line);
	void CheckCompileError(GLint Task, Components Component);

	GLfloat Vertices[16]{};
	GLuint VertexArrayObject{};
	GLuint VertexBufferObject{};
	GLuint PixelBufferTexture{};
};

inline void GFX_OpenGLShaderClass::LoadShader(const std::string& ShaderName)
{
	// Set texture coordinates
	// Top-Left
	Vertices[2] = 0.0F;
	Vertices[3] = 0.0F;

	// Top-Right
	Vertices[6] = 1.0F;
	Vertices[7] = 0.0F;

	// Bottom-Right
	Vertices[10] = 1.0F;
	Vertices[11] = 1.0F;

	// Bottom-Left
	Vertices[14] = 0.0F;
	Vertices[15] = 1.0F;

	constexpr GLint Elements[] {
		0, 1, 2,
		2, 3, 0
	};

	GLuint ElementBufferObject{};

	Tools_Console::DisplayText(BRIGHT_MAGENTA, fmt::format("\nBuilding shader program {}\n", ShaderName));

	// Create buffers
	glGenBuffers(1, &VertexBufferObject);
	glCheckError();
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);
	glCheckError();
	glBufferData(GL_ARRAY_BUFFER, 2048, nullptr, GL_DYNAMIC_DRAW);
	glCheckError();
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);
	glCheckError();

	glGenVertexArrays(1, &VertexArrayObject);
	glCheckError();
	glBindVertexArray(VertexArrayObject);
	glCheckError();

	glGenBuffers(1, &ElementBufferObject);
	glCheckError();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBufferObject);
	glCheckError();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Elements), Elements, GL_STATIC_DRAW);
	glCheckError();

	// Create and compile the vertex shader
	Tools_Console::DisplayText(BRIGHT_MAGENTA, "Loading and compiling vertex shader...");
	const GLchar* VertexShaderSource{ LoadShaderSource(fmt::format("./Shader/Vertex/{}.vert", ShaderName)) };
	const GLint VertexShader{ glCreateShader(GL_VERTEX_SHADER) };
	glCheckError();
	glShaderSource(VertexShader, 1, &VertexShaderSource, nullptr);
	glCheckError();
	glCompileShader(VertexShader);
	glCheckError();
	CheckCompileError(VertexShader, Components::Shader);

	// Create and compile the fragment shader
	Tools_Console::DisplayText(BRIGHT_MAGENTA, "Loading and compiling fragment shader...");
	const GLchar* FragmentShaderSource{ LoadShaderSource(fmt::format("./Shader/Fragment/{}.frag", ShaderName)) };
	const GLint FragmentShader{ glCreateShader(GL_FRAGMENT_SHADER) };
	glCheckError();
	glShaderSource(FragmentShader, 1, &FragmentShaderSource, nullptr);
	glCheckError();
	glCompileShader(FragmentShader);
	glCheckError();
	CheckCompileError(FragmentShader, Components::Shader);

	// Link the vertex and fragment shader into a shader program
	Tools_Console::DisplayText(BRIGHT_MAGENTA, "Linking shader program...");
	const GLint ShaderProgram{ glCreateProgram() };
	glCheckError();
	glAttachShader(ShaderProgram, VertexShader);
	glCheckError();
	glAttachShader(ShaderProgram, FragmentShader);
	glCheckError();
	glBindFragDataLocation(ShaderProgram, 0, "outColor");
	glCheckError();
	glLinkProgram(ShaderProgram);
	glCheckError();

	// Check shader program
	CheckCompileError(ShaderProgram, Components::Program);

	// Everything´s fine, now we can use the shader program...
	glUseProgram(ShaderProgram);
	glCheckError();

	// Specify the layout of the vertex data
	const GLint PositionAttrib{ glGetAttribLocation(ShaderProgram, "position") };
	glCheckError();
	glEnableVertexAttribArray(PositionAttrib);
	glCheckError();
	glVertexAttribPointer(PositionAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
	glCheckError();

	const GLint TextureAttrib{ glGetAttribLocation(ShaderProgram, "texcoord") };
	glCheckError();
	glEnableVertexAttribArray(TextureAttrib);
	glCheckError();
	glVertexAttribPointer(TextureAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), reinterpret_cast<GLvoid*>(2 * sizeof(GLfloat))); //-V566
	glCheckError();

	float ProjectionMatrix[16];
	Ortho2D(ProjectionMatrix, 0.0F, static_cast<GLfloat>(lwmf::ViewportWidth), static_cast<GLfloat>(lwmf::ViewportHeight), 0.0F);
	const GLint Projection{ glGetUniformLocation(ShaderProgram, "MVP") };
	glCheckError();
	glUniformMatrix4fv(Projection, 1, GL_FALSE, ProjectionMatrix);
	glCheckError();

	// Since the shader program is now loaded into GPU, we can delete the shader program...
	glDetachShader(ShaderProgram, FragmentShader);
	glCheckError();
	glDetachShader(ShaderProgram, VertexShader);
	glCheckError();
	glDeleteShader(FragmentShader);
	glCheckError();
	glDeleteShader(VertexShader);
	glCheckError();

	glEnable(GL_BLEND);
	glCheckError();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCheckError();
}

inline void GFX_OpenGLShaderClass::LoadTextureInGPU(const lwmf::TextureStruct& TextureData, GLuint* Texture)
{
	glGenTextures(1, Texture);
	glCheckError();
	glBindTexture(GL_TEXTURE_2D, *Texture);
	glCheckError();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, TextureData.Width, TextureData.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, TextureData.Texture.data());
	glCheckError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glCheckError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glCheckError();
}

inline void GFX_OpenGLShaderClass::LoadSurfaceInGPU(const SDL_Surface* Surface, GLuint* Texture)
{
	glGenTextures(1, Texture);
	glCheckError();
	glBindTexture(GL_TEXTURE_2D, *Texture);
	glCheckError();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Surface->w, Surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, Surface->pixels);
	glCheckError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glCheckError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glCheckError();
}

inline void GFX_OpenGLShaderClass::RenderTexture(const GLuint* Texture, const std::int_fast32_t PosX, const std::int_fast32_t PosY, const std::int_fast32_t Width, const std::int_fast32_t Height)
{
	UpdateVertices(PosX, PosY, Width, Height);
	glBindTexture(GL_TEXTURE_2D, *Texture);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

inline void GFX_OpenGLShaderClass::LoadStaticTextureInGPU(const lwmf::TextureStruct& TextureData, GLuint* Texture, const std::int_fast32_t PosX, const std::int_fast32_t PosY, const std::int_fast32_t Width, const std::int_fast32_t Height)
{
	UpdateVertices(PosX, PosY, Width, Height);
	LoadTextureInGPU(TextureData, Texture);
}

inline void GFX_OpenGLShaderClass::RenderStaticTexture(const GLuint* Texture)
{
	glBindVertexArray(VertexArrayObject);
	glBindTexture(GL_TEXTURE_2D, *Texture);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

inline void GFX_OpenGLShaderClass::PreparePixelBufferTexture(const std::int_fast32_t PosX, const std::int_fast32_t PosY, const std::int_fast32_t Width, const std::int_fast32_t Height)
{
	UpdateVertices(PosX, PosY, Width, Height);
	glCheckError();
	glGenTextures(1, &PixelBufferTexture);
	glCheckError();
	glBindTexture(GL_TEXTURE_2D, PixelBufferTexture);
	glCheckError();
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, Width, Height);
	glCheckError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glCheckError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glCheckError();
}

inline void GFX_OpenGLShaderClass::RenderPixelBufferTexture()
{
	glBindVertexArray(VertexArrayObject);
	glBindTexture(GL_TEXTURE_2D, PixelBufferTexture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,lwmf::ViewportWidth, lwmf::ViewportHeight, GL_RGBA, GL_UNSIGNED_BYTE, lwmf::PixelBuffer.data());
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

inline void GFX_OpenGLShaderClass::Ortho2D(GLfloat* Matrix, const GLfloat Left, const GLfloat Right, const GLfloat Bottom, const GLfloat Top)
{
	const GLfloat InvY{ 1.0F / (Top - Bottom) };
	const GLfloat InvX{ 1.0F / (Right - Left) };

	// First column
	*Matrix++ = 2.0F * InvX;
	*Matrix++ = 0.0F;
	*Matrix++ = 0.0F;
	*Matrix++ = 0.0F;

	// Second
	*Matrix++ = 0.0F;
	*Matrix++ = 2.0F * InvY;
	*Matrix++ = 0.0F;
	*Matrix++ = 0.0F;

	// Third
	*Matrix++ = 0.0F;
	*Matrix++ = 0.0F;
	*Matrix++ = -1.0F;
	*Matrix++ = 0.0F;

	// Fourth
	*Matrix++ = -(Right + Left) * InvX;
	*Matrix++ = -(Top + Bottom) * InvY;
	*Matrix++ = 0.0F;
	*Matrix++ = 1.0F;
}

inline void GFX_OpenGLShaderClass::UpdateVertices(const std::int_fast32_t PosX, const std::int_fast32_t PosY, const std::int_fast32_t Width, const std::int_fast32_t Height)
{
	glBindVertexArray(VertexArrayObject);
	glCheckError();
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);
	glCheckError();

	Vertices[0] = static_cast<GLfloat>(PosX);
	Vertices[1] = static_cast<GLfloat>(PosY);

	Vertices[4] = static_cast<GLfloat>(PosX + Width);
	Vertices[5] = static_cast<GLfloat>(PosY);

	Vertices[8] = static_cast<GLfloat>(PosX + Width);
	Vertices[9] = static_cast<GLfloat>(PosY + Height);

	Vertices[12] = static_cast<GLfloat>(PosX);
	Vertices[13] = static_cast<GLfloat>(PosY + Height);

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof Vertices, Vertices);
	glCheckError();
}

inline const char* GFX_OpenGLShaderClass::LoadShaderSource(const std::string& FileName)
{
	std::stringstream Result;

	if (Tools_ErrorHandling::CheckFileExistence(FileName, HideMessage, StopOnError))
	{
		std::ifstream ShaderFile(FileName);

		ShaderFile.ignore((std::numeric_limits<std::streamsize>::max)());
		ShaderFile.clear();
		ShaderFile.seekg(0, std::ios_base::beg);

		Result << ShaderFile.rdbuf();
	}

	return Result.str().c_str();
}

inline void GFX_OpenGLShaderClass::CheckError(const std::int_fast32_t Line)
{
	if (Debug)
	{
		GLenum ErrorCode{};

		while ((ErrorCode = glGetError()) != GL_NO_ERROR)
		{
			std::string Error;

			switch (ErrorCode)
			{
				case GL_INVALID_ENUM:
				{
					Error = "INVALID_ENUM";
					break;
				}
				case GL_INVALID_VALUE:
				{
					Error = "INVALID_VALUE";
					break;
				}
				case GL_INVALID_OPERATION:
				{
					Error = "INVALID_OPERATION";
					break;
				}
				case GL_STACK_OVERFLOW:
				{
					Error = "STACK_OVERFLOW";
					break;
				}
				case GL_STACK_UNDERFLOW:
				{
					Error = "STACK_UNDERFLOW";
					break;
				}
				case GL_OUT_OF_MEMORY:
				{
					Error = "OUT_OF_MEMORY";
					break;
				}
				case GL_INVALID_FRAMEBUFFER_OPERATION:
				{
					Error = "INVALID_FRAMEBUFFER_OPERATION";
					break;
				}
				default: {}
			}

			Tools_ErrorHandling::DisplayError(fmt::format("OpenGL error {0} in line {1}!", Error, Line));
		}
	}
}

inline void GFX_OpenGLShaderClass::CheckCompileError(const GLint Task, const Components Component)
{
	if (Debug)
	{
		GLint ErrorResult{};
		GLchar ErrorLog[512];

		switch (Component)
		{
			case Components::Shader:
			{
				glGetShaderiv(Task, GL_COMPILE_STATUS, &ErrorResult);
				glCheckError();
				ErrorResult == GL_FALSE ? (glGetShaderInfoLog(Task, 512, nullptr, ErrorLog), Tools_ErrorHandling::DisplayError(fmt::format("\n{}\nERROR!", ErrorLog))) : Tools_ErrorHandling::DisplayOK();
				glCheckError();
				break;
			}
			case Components::Program:
			{
				glGetProgramiv(Task, GL_LINK_STATUS, &ErrorResult);
				glCheckError();
				ErrorResult == GL_FALSE ? (glGetProgramInfoLog(Task, 512, nullptr, ErrorLog), Tools_ErrorHandling::DisplayError(fmt::format("\n{}\nERROR!", ErrorLog))) : Tools_ErrorHandling::DisplayOK();
				glCheckError();
				break;
			}
			default: {}
		}
	}
}