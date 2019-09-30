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

#define glCheckError() CheckError(__LINE__)

#include <cstdint>
#include <string>
#include <vector>
#include <fstream>

#include "lwmf_logging.hpp"
#include "lwmf_texture.hpp"

namespace lwmf
{


	class ShaderClass final
	{
	public:
		void LoadShader(const std::string& ShaderName, const TextureStruct& Texture);
		void LoadTextureInGPU(const TextureStruct& Texture, GLuint* TextureID);
		void RenderTexture(const GLuint* Texture, std::int_fast32_t PosX, std::int_fast32_t PosY, std::int_fast32_t Width, std::int_fast32_t Height, bool Blend, float Opacity);
		void LoadStaticTextureInGPU(const TextureStruct& Texture, GLuint* TextureID, std::int_fast32_t PosX, std::int_fast32_t PosY, std::int_fast32_t Width, std::int_fast32_t Height);
		void RenderStaticTexture(const GLuint* TextureID, bool Blend, float Opacity);
		void PrepareLWMFTexture(const TextureStruct& Texture, std::int_fast32_t PosX, std::int_fast32_t PosY);
		void RenderLWMFTexture(const TextureStruct& Texture, bool Blend, float Opacity);

	private:
		enum class Components : std::int_fast32_t
		{
			Shader,
			Program
		};

		void Ortho2D(std::vector<GLfloat>& Matrix, GLfloat Left, GLfloat Right, GLfloat Bottom, GLfloat Top);
		void UpdateVertices(std::int_fast32_t PosX, std::int_fast32_t PosY, std::int_fast32_t Width, std::int_fast32_t Height);
		std::string LoadShaderSource(const std::string& FileName, const std::string& ShaderName);
		void CheckError(std::int_fast32_t Line);
		void CheckCompileError(GLuint Task, Components Component);

		std::vector<GLfloat> Vertices;
		GLint OpacityLocation{};
		GLuint ShaderProgram{};
		GLuint VertexArrayObject{};
		GLuint VertexBufferObject{};
		GLuint LWMFTextureID{};
	};

	inline void ShaderClass::LoadShader(const std::string& ShaderName, const TextureStruct& Texture)
	{
		const std::string VertexShaderPath{ "./Shader/" };
		const std::string FragmentShaderPath{ "./Shader/" };
		const std::string VertexShaderFileSuffix{ ".vert" };
		const std::string FragmentShaderFileSuffix{ ".frag" };
		const std::string ShaderNameString{ "(Shadername " + ShaderName + ") - " };

		Vertices.resize(16);

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

		std::vector<GLint> Elements
		{
			0, 1, 2,
			2, 3, 0
		};

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, ShaderNameString + "Create vertex buffer object...");
		glGenBuffers(1, &VertexBufferObject);
		glCheckError();
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);
		glCheckError();
		glBufferData(GL_ARRAY_BUFFER, 2048, nullptr, GL_DYNAMIC_DRAW);
		glCheckError();
		glBufferSubData(GL_ARRAY_BUFFER, 0, Vertices.size() * sizeof(GLfloat), Vertices.data());
		glCheckError();

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, ShaderNameString + "Create vertex array object...");
		glGenVertexArrays(1, &VertexArrayObject);
		glCheckError();
		glBindVertexArray(VertexArrayObject);
		glCheckError();

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, ShaderNameString + "Create element buffer object...");
		GLuint ElementBufferObject{};
		glGenBuffers(1, &ElementBufferObject);
		glCheckError();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBufferObject);
		glCheckError();
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, Elements.size() * sizeof(GLint), Elements.data(), GL_STATIC_DRAW);
		glCheckError();

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, ShaderNameString + "Create and compile the vertex shader...");
		const std::string VertexShaderString{ LoadShaderSource(VertexShaderPath + ShaderName + VertexShaderFileSuffix, ShaderName) };
		const GLchar* VertexShaderSource{ VertexShaderString.c_str() };
		const GLint VertexShaderSourceLength{ static_cast<GLint>(VertexShaderString.size()) };
		const GLuint VertexShader{ static_cast<GLuint>(glCreateShader(GL_VERTEX_SHADER)) };
		glCheckError();
		glShaderSource(VertexShader, 1, &VertexShaderSource, &VertexShaderSourceLength);
		glCheckError();
		glCompileShader(VertexShader);
		glCheckError();
		CheckCompileError(VertexShader, Components::Shader);

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, ShaderNameString + "Create and compile the fragment shader...");
		const std::string FragmentShaderString{ LoadShaderSource(FragmentShaderPath + ShaderName + FragmentShaderFileSuffix, ShaderName) };
		const GLchar* FragmentShaderSource{ FragmentShaderString.c_str() };
		const GLint FragmentShaderSourceLength{ static_cast<GLint>(FragmentShaderString.size()) };
		const GLuint FragmentShader{ static_cast<GLuint>(glCreateShader(GL_FRAGMENT_SHADER)) };
		glCheckError();
		glShaderSource(FragmentShader, 1, &FragmentShaderSource, &FragmentShaderSourceLength);
		glCheckError();
		glCompileShader(FragmentShader);
		glCheckError();
		CheckCompileError(FragmentShader, Components::Shader);

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, ShaderNameString + "Link the vertex and fragment shader into a shader program...");
		ShaderProgram = static_cast<GLuint>(glCreateProgram());
		glCheckError();
		glAttachShader(ShaderProgram, VertexShader);
		glCheckError();
		glAttachShader(ShaderProgram, FragmentShader);
		glCheckError();
		glBindFragDataLocation(ShaderProgram, 0, "outColor");
		glCheckError();
		glLinkProgram(ShaderProgram);
		glCheckError();

		CheckCompileError(ShaderProgram, Components::Program);

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, ShaderNameString + "Use shader program...");
		glUseProgram(ShaderProgram);
		glCheckError();

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, ShaderNameString + "Specify the layout of the vertex data...");
		const GLuint PositionAttrib{ static_cast<GLuint>(glGetAttribLocation(ShaderProgram, "position")) };
		glCheckError();
		glEnableVertexAttribArray(PositionAttrib);
		glCheckError();
		glVertexAttribPointer(PositionAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
		glCheckError();

		const GLuint TextureAttrib{ static_cast<GLuint>(glGetAttribLocation(ShaderProgram, "texcoord")) };
		glCheckError();
		glEnableVertexAttribArray(TextureAttrib);
		glCheckError();
		glVertexAttribPointer(TextureAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), reinterpret_cast<char*>(0 + (2 * sizeof(GLfloat))));
		glCheckError();

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, ShaderNameString + "Create projection matrix...");
		std::vector<GLfloat> ProjectionMatrix(16);
		Ortho2D(ProjectionMatrix, 0.0F, static_cast<GLfloat>(Texture.Width), static_cast<GLfloat>(Texture.Height), 0.0F);
		glCheckError();
		const GLint Projection{ glGetUniformLocation(ShaderProgram, "MVP") };
		glCheckError();
		glUniformMatrix4fv(Projection, 1, GL_FALSE, ProjectionMatrix.data());
		glCheckError();

		OpacityLocation = glGetUniformLocation(ShaderProgram, "Opacity");
		glCheckError();

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, ShaderNameString + "Since the shader program is now loaded into GPU, we can delete the shaders...");
		glDetachShader(ShaderProgram, FragmentShader);
		glCheckError();
		glDetachShader(ShaderProgram, VertexShader);
		glCheckError();
		glDeleteShader(FragmentShader);
		glCheckError();
		glDeleteShader(VertexShader);
		glCheckError();

		// Set some flags
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glCheckError();
		glDisable(GL_DEPTH_TEST);
		glCheckError();
		glDisable(GL_DITHER);
		glCheckError();
	}

	inline void ShaderClass::LoadTextureInGPU(const lwmf::TextureStruct& Texture, GLuint *TextureID)
	{
		glGenTextures(1, TextureID);
		glCheckError();
		glBindTexture(GL_TEXTURE_2D, *TextureID);
		glCheckError();
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Texture.Width, Texture.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Texture.Pixels.data());
		glCheckError();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glCheckError();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glCheckError();
	}

	inline void ShaderClass::RenderTexture(const GLuint* TextureID, const std::int_fast32_t PosX, const std::int_fast32_t PosY, const std::int_fast32_t Width, const std::int_fast32_t Height, const bool Blend, const float Opacity)
	{
		if (Blend)
		{
			glEnable(GL_BLEND);
		}

		glUseProgram(ShaderProgram);
		glUniform1f(OpacityLocation, Opacity);
		UpdateVertices(PosX, PosY, Width, Height);
		glBindTexture(GL_TEXTURE_2D, *TextureID);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

		if (Blend)
		{
			glDisable(GL_BLEND);
		}
	}

	inline void ShaderClass::LoadStaticTextureInGPU(const lwmf::TextureStruct& Texture, GLuint* TextureID, const std::int_fast32_t PosX, const std::int_fast32_t PosY, const std::int_fast32_t Width, const std::int_fast32_t Height)
	{
		UpdateVertices(PosX, PosY, Width, Height);
		LoadTextureInGPU(Texture, TextureID);
	}

	inline void ShaderClass::RenderStaticTexture(const GLuint* TextureID, const bool Blend, const float Opacity)
	{
		if (Blend)
		{
			glEnable(GL_BLEND);
		}

		glUseProgram(ShaderProgram);
		glUniform1f(OpacityLocation, Opacity);
		glBindVertexArray(VertexArrayObject);
		glBindTexture(GL_TEXTURE_2D, *TextureID);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

		if (Blend)
		{
			glDisable(GL_BLEND);
		}
	}

	inline void ShaderClass::PrepareLWMFTexture(const lwmf::TextureStruct& Texture, const std::int_fast32_t PosX, const std::int_fast32_t PosY)
	{
		UpdateVertices(PosX, PosY, Texture.Width, Texture.Height);
		glGenTextures(1, &LWMFTextureID);
		glCheckError();
		glBindTexture(GL_TEXTURE_2D, LWMFTextureID);
		glCheckError();

		if (FullscreenFlag)
		{
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, Texture.Width, Texture.Height);
			glCheckError();
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glCheckError();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glCheckError();
	}

	inline void ShaderClass::RenderLWMFTexture(const lwmf::TextureStruct& Texture, const bool Blend, const float Opacity)
	{
		if (Blend)
		{
			glEnable(GL_BLEND);
		}

		glUseProgram(ShaderProgram);
		glUniform1f(OpacityLocation, Opacity);
		glBindVertexArray(VertexArrayObject);
		glBindTexture(GL_TEXTURE_2D, LWMFTextureID);
		FullscreenFlag ? glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Texture.Width, Texture.Height, GL_RGBA, GL_UNSIGNED_BYTE, Texture.Pixels.data()) : glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Texture.Width, Texture.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Texture.Pixels.data());
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

		if (Blend)
		{
			glDisable(GL_BLEND);
		}
	}

	inline void ShaderClass::Ortho2D(std::vector<GLfloat>& Matrix, const GLfloat Left, const GLfloat Right, const GLfloat Bottom, const GLfloat Top)
	{
		const GLfloat InvY{ 1.0F / (Top - Bottom) };
		const GLfloat InvX{ 1.0F / (Right - Left) };

		// First column
		Matrix[0] = 2.0F * InvX;
		Matrix[1] = 0.0F;
		Matrix[2] = 0.0F;
		Matrix[3] = 0.0F;

		// Second
		Matrix[4] = 0.0F;
		Matrix[5] = 2.0F * InvY;
		Matrix[6] = 0.0F;
		Matrix[7] = 0.0F;

		// Third
		Matrix[8] = 0.0F;
		Matrix[9] = 0.0F;
		Matrix[10] = -1.0F;
		Matrix[11] = 0.0F;

		// Fourth
		Matrix[12] = -(Right + Left) * InvX;
		Matrix[13] = -(Top + Bottom) * InvY;
		Matrix[14] = 0.0F;
		Matrix[15] = 1.0F;
	}

	inline void ShaderClass::UpdateVertices(const std::int_fast32_t PosX, const std::int_fast32_t PosY, const std::int_fast32_t Width, const std::int_fast32_t Height)
	{
		glBindVertexArray(VertexArrayObject);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);

		Vertices[0] = static_cast<GLfloat>(PosX);
		Vertices[1] = static_cast<GLfloat>(PosY);

		Vertices[4] = static_cast<GLfloat>(PosX + Width);
		Vertices[5] = static_cast<GLfloat>(PosY);

		Vertices[8] = static_cast<GLfloat>(PosX + Width);
		Vertices[9] = static_cast<GLfloat>(PosY + Height);

		Vertices[12] = static_cast<GLfloat>(PosX);
		Vertices[13] = static_cast<GLfloat>(PosY + Height);

		glBufferSubData(GL_ARRAY_BUFFER, 0, Vertices.size() * sizeof(GLfloat), Vertices.data());
	}

	inline std::string ShaderClass::LoadShaderSource(const std::string& FileName, const std::string& ShaderName)
	{
		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "(Shadername " + ShaderName + ") - Loading shaderfile " + FileName);

		std::ifstream ShaderFile(FileName, std::ios::in);
		std::string Result;

		if (ShaderFile.fail())
		{
			LWMFSystemLog.AddEntry(LogLevel::Critical, __FILENAME__, "(Shadername " + ShaderName + ") - Loading of shaderfile " + FileName + " failed!");
		}
		else
		{
			std::string Line;

			while (std::getline(ShaderFile, Line))
			{
				Result += Line + "\n";
			}
		}

		return Result;
	}

	inline void ShaderClass::CheckError(const std::int_fast32_t Line)
	{
		// https://www.khronos.org/opengl/wiki/OpenGL_Error

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
				case GL_CONTEXT_LOST:
				{
					Error = "GL_CONTEXT_LOST";
					break;
				}
				default: {}
			}

			LWMFSystemLog.AddEntry(LogLevel::Critical, __FILENAME__, "OpenGL error " + Error + " in line " + std::to_string(Line) + "!");
		}
	}

	inline void ShaderClass::CheckCompileError(const GLuint Task, const Components Component)
	{
		GLint ErrorResult{};
		std::vector<GLchar> ErrorLog(512);

		switch (Component)
		{
			case Components::Shader:
			{
				glGetShaderiv(Task, GL_COMPILE_STATUS, &ErrorResult);
				glCheckError();

				if (ErrorResult == GL_FALSE)
				{
					glGetShaderInfoLog(Task, 512, nullptr, ErrorLog.data());
					glCheckError();
					LWMFSystemLog.AddEntry(LogLevel::Critical, __FILENAME__, std::string(ErrorLog.data()));
				}

				break;
			}
			case Components::Program:
			{
				glGetProgramiv(Task, GL_LINK_STATUS, &ErrorResult);
				glCheckError();

				if (ErrorResult == GL_FALSE)
				{
					glGetProgramInfoLog(Task, 512, nullptr, ErrorLog.data());
					glCheckError();
					LWMFSystemLog.AddEntry(LogLevel::Critical, __FILENAME__, std::string(ErrorLog.data()));
				}

				break;
			}
			default: {}
		}
	}


} // namespace lwmf