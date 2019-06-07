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
#include <fstream>
#include <limits>
#include <sstream>

#include "lwmf_logging.hpp"
#include "lwmf_texture.hpp"

namespace lwmf
{


	class ShaderClass final
	{
	public:
		void LoadShader(const std::string& ShaderName, const TextureStruct& Texture);
		void LoadTextureInGPU(const TextureStruct& Texture, GLuint* TextureID);
		void RenderTexture(const GLuint* Texture, std::int_fast32_t PosX, std::int_fast32_t PosY, std::int_fast32_t Width, std::int_fast32_t Height);
		void LoadStaticTextureInGPU(const TextureStruct& Texture, GLuint* TextureID, std::int_fast32_t PosX, std::int_fast32_t PosY, std::int_fast32_t Width, std::int_fast32_t Height);
		void RenderStaticTexture(const GLuint* TextureID);
		void PrepareLWMFTexture(const TextureStruct& Texture, std::int_fast32_t PosX, std::int_fast32_t PosY);
		void RenderLWMFTexture(const TextureStruct& Texture);

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
		GLuint LWMFTextureID{};
	};

	inline void ShaderClass::LoadShader(const std::string& ShaderName, const TextureStruct& Texture)
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

		constexpr GLint Elements[]{
			0, 1, 2,
			2, 3, 0
		};

		GLuint ElementBufferObject{};

		lwmf_SystemLog.AddEntry("SHADER (" + ShaderName + "): Create vertex buffer object...");
		glGenBuffers(1, &VertexBufferObject);
		glCheckError();
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);
		glCheckError();
		glBufferData(GL_ARRAY_BUFFER, 2048, nullptr, GL_DYNAMIC_DRAW);
		glCheckError();
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);
		glCheckError();

		lwmf_SystemLog.AddEntry("SHADER (" + ShaderName + "): Create vertex array object...");
		glGenVertexArrays(1, &VertexArrayObject);
		glCheckError();
		glBindVertexArray(VertexArrayObject);
		glCheckError();

		lwmf_SystemLog.AddEntry("SHADER (" + ShaderName + "): Create element buffer object...");
		glGenBuffers(1, &ElementBufferObject);
		glCheckError();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBufferObject);
		glCheckError();
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Elements), Elements, GL_STATIC_DRAW);
		glCheckError();

		lwmf_SystemLog.AddEntry("SHADER (" + ShaderName + "): Create and compile the vertex shader...");
		const GLchar* VertexShaderSource{ LoadShaderSource("./Shader/Vertex/" + ShaderName + ".vert") };
		const GLint VertexShader{ glCreateShader(GL_VERTEX_SHADER) };
		glCheckError();
		glShaderSource(VertexShader, 1, &VertexShaderSource, nullptr);
		glCheckError();
		glCompileShader(VertexShader);
		glCheckError();
		CheckCompileError(VertexShader, Components::Shader);

		lwmf_SystemLog.AddEntry("SHADER (" + ShaderName + "): Create and compile the fragment shader...");
		const GLchar* FragmentShaderSource{ LoadShaderSource("./Shader/Fragment/" + ShaderName + ".frag") };
		const GLint FragmentShader{ glCreateShader(GL_FRAGMENT_SHADER) };
		glCheckError();
		glShaderSource(FragmentShader, 1, &FragmentShaderSource, nullptr);
		glCheckError();
		glCompileShader(FragmentShader);
		glCheckError();
		CheckCompileError(FragmentShader, Components::Shader);

		lwmf_SystemLog.AddEntry("SHADER (" + ShaderName + "): Link the vertex and fragment shader into a shader program...");
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

		CheckCompileError(ShaderProgram, Components::Program);

		lwmf_SystemLog.AddEntry("SHADER (" + ShaderName + "): Use shader program...");
		glUseProgram(ShaderProgram);
		glCheckError();

		lwmf_SystemLog.AddEntry("SHADER (" + ShaderName + "): Specify the layout of the vertex data...");
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

		lwmf_SystemLog.AddEntry("SHADER (" + ShaderName + "): Create projection matrix...");
		GLfloat ProjectionMatrix[16]{};
		Ortho2D(ProjectionMatrix, 0.0F, static_cast<GLfloat>(Texture.Width), static_cast<GLfloat>(Texture.Height), 0.0F);
		glCheckError();
		const GLint Projection{ glGetUniformLocation(ShaderProgram, "MVP") };
		glCheckError();
		glUniformMatrix4fv(Projection, 1, GL_FALSE, ProjectionMatrix);
		glCheckError();

		lwmf_SystemLog.AddEntry("SHADER (" + ShaderName + "): Since the shader program is now loaded into GPU, we can delete the shader program...");
		glDetachShader(ShaderProgram, FragmentShader);
		glCheckError();
		glDetachShader(ShaderProgram, VertexShader);
		glCheckError();
		glDeleteShader(FragmentShader);
		glCheckError();
		glDeleteShader(VertexShader);
		glCheckError();
	}

	inline void ShaderClass::LoadTextureInGPU(const lwmf::TextureStruct& Texture, GLuint* TextureID)
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

	inline void ShaderClass::RenderTexture(const GLuint* TextureID, const std::int_fast32_t PosX, const std::int_fast32_t PosY, const std::int_fast32_t Width, const std::int_fast32_t Height)
	{
		UpdateVertices(PosX, PosY, Width, Height);
		glBindTexture(GL_TEXTURE_2D, *TextureID);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	}

	inline void ShaderClass::LoadStaticTextureInGPU(const lwmf::TextureStruct& Texture, GLuint* TextureID, const std::int_fast32_t PosX, const std::int_fast32_t PosY, const std::int_fast32_t Width, const std::int_fast32_t Height)
	{
		UpdateVertices(PosX, PosY, Width, Height);
		LoadTextureInGPU(Texture, TextureID);
	}

	inline void ShaderClass::RenderStaticTexture(const GLuint* TextureID)
	{
		glBindVertexArray(VertexArrayObject);
		glBindTexture(GL_TEXTURE_2D, *TextureID);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	}

	inline void ShaderClass::PrepareLWMFTexture(const lwmf::TextureStruct& Texture, const std::int_fast32_t PosX, const std::int_fast32_t PosY)
	{
		UpdateVertices(PosX, PosY, Texture.Width, Texture.Height);
		glGenTextures(1, &LWMFTextureID);
		glCheckError();
		glBindTexture(GL_TEXTURE_2D, LWMFTextureID);
		glCheckError();

		if (FullscreenFlag == 1)
		{
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, Texture.Width, Texture.Height);
			glCheckError();
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glCheckError();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glCheckError();
	}

	inline void ShaderClass::RenderLWMFTexture(const lwmf::TextureStruct& Texture)
	{
		glBindVertexArray(VertexArrayObject);
		glBindTexture(GL_TEXTURE_2D, LWMFTextureID);
		FullscreenFlag == 1 ? glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Texture.Width, Texture.Height, GL_RGBA, GL_UNSIGNED_BYTE, Texture.Pixels.data()) : glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Texture.Width, Texture.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Texture.Pixels.data());
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	}

	inline void ShaderClass::Ortho2D(GLfloat* Matrix, const GLfloat Left, const GLfloat Right, const GLfloat Bottom, const GLfloat Top)
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

		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);
	}

	inline const char* ShaderClass::LoadShaderSource(const std::string& FileName)
	{
		std::stringstream Result;
		std::ifstream ShaderFile(FileName);

		ShaderFile.ignore((std::numeric_limits<std::streamsize>::max)());
		ShaderFile.clear();
		ShaderFile.seekg(0, std::ios_base::beg);

		Result << ShaderFile.rdbuf();
		return Result.str().c_str();
	}

	inline void ShaderClass::CheckError(const std::int_fast32_t Line)
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

			lwmf_SystemLog.LogErrorAndThrowException("OpenGL error " + Error + " in line " + std::to_string(Line) + "!");
		}
	}

	inline void ShaderClass::CheckCompileError(const GLint Task, const Components Component)
	{
		GLint ErrorResult{};
		GLchar ErrorLog[512];

		switch (Component)
		{
			case Components::Shader:
			{
				glGetShaderiv(Task, GL_COMPILE_STATUS, &ErrorResult);

				if (ErrorResult == GL_FALSE)
				{
					glGetShaderInfoLog(Task, 512, nullptr, ErrorLog);
					lwmf_SystemLog.LogErrorAndThrowException(std::string(ErrorLog));
				}

				break;
			}
			case Components::Program:
			{
				glGetProgramiv(Task, GL_LINK_STATUS, &ErrorResult);

				if (ErrorResult == GL_FALSE)
				{
					glGetProgramInfoLog(Task, 512, nullptr, ErrorLog);
					lwmf_SystemLog.LogErrorAndThrowException(std::string(ErrorLog));
				}

				break;
			}
			default: {}
		}
	}


} // namespace lwmf