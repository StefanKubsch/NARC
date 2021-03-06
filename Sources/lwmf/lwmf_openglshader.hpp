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
#include <array>
#include <map>
#include <cstring>

#include "lwmf_logging.hpp"
#include "lwmf_texture.hpp"

namespace lwmf
{


	//
	// Shader source codes
	//

	inline const std::string DefaultFragmentShaderSource
	{
		"#version 430 core\n"
		"in vec2 Texcoord;\n"
		"out vec4 outColor;\n"
		"uniform float Opacity;\n"
		"uniform sampler2D Texture;\n"
		"void main()\n"
		"{\n"
		"outColor = texture(Texture, Texcoord);\n"
		"outColor.a *= Opacity;\n"
		"}"
	};

	inline const std::string DefaultVertexShaderSource
	{
		"#version 430 core\n"
		"in vec2 position;\n"
		"in vec2 texcoord;\n"
		"out vec2 Texcoord;\n"
		"uniform mat4 MVP;\n"
		"void main()\n"
		"{\n"
		"Texcoord = texcoord;\n"
		"gl_Position = MVP * vec4(position, 0.0f, 1.0f);\n"
		"}"
	};

	//
	// OpenGL class
	//

	class ShaderClass final
	{
	public:
		void LoadShader(const std::string& ShaderName, const TextureStruct& Texture);
		static void LoadTextureInGPU(const TextureStruct& Texture, GLuint* TextureID);
		void RenderTexture(const GLuint* Texture, std::int_fast32_t PosX, std::int_fast32_t PosY, std::int_fast32_t Width, std::int_fast32_t Height, bool Blend, float Opacity);
		void LoadStaticTextureInGPU(const TextureStruct& Texture, GLuint* TextureID, std::int_fast32_t PosX, std::int_fast32_t PosY, std::int_fast32_t Width, std::int_fast32_t Height);
		void RenderStaticTexture(const GLuint* TextureID, bool Blend, float Opacity);
		void PrepareLWMFTexture(const TextureStruct& Texture, std::int_fast32_t PosX, std::int_fast32_t PosY);
		void RenderLWMFTexture(const TextureStruct& Texture, bool Blend, float Opacity);

		GLuint OGLTextureID{};

	private:
		enum class Components : std::int_fast32_t
		{
			Shader,
			Program
		};

		static void Ortho2D(std::array<GLfloat, 16>& Matrix, GLfloat Left, GLfloat Right, GLfloat Bottom, GLfloat Top);
		void UpdateVertices(std::int_fast32_t PosX, std::int_fast32_t PosY, std::int_fast32_t Width, std::int_fast32_t Height);
		static std::string LoadShaderSource(const std::string& SourceName);
		static void CheckError(std::int_fast32_t Line);
		static void CheckCompileError(GLuint Task, Components Component);

		std::array<GLfloat, 16> Vertices
		{
			0.0F, 0.0F,
			0.0F, 0.0F, // Top-Left
			0.0F, 0.0F,
			1.0F, 0.0F, // Top-Right
			0.0F, 0.0F,
			1.0F, 1.0F, // Bottom-Right
			0.0F, 0.0F,
			0.0F, 1.0F  // Bottom-Left
		};

		GLint OpacityLocation{};
		GLuint ShaderProgram{};
		GLuint VertexArrayObject{};
		GLuint VertexBufferObject{};
	};

	inline void ShaderClass::LoadShader(const std::string& ShaderName, const TextureStruct& Texture)
	{
		const std::string ShaderNameString{ "(Shadername " + ShaderName + ") - " };

		constexpr std::array<GLint, 6> Elements
		{
			0, 1, 2,
			2, 3, 0
		};

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, ShaderNameString + "Create vertex buffer object...");
		glGenBuffers(1, &VertexBufferObject);
		glCheckError();
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);
		glCheckError();
		glBufferData(GL_ARRAY_BUFFER, 2048, nullptr, GL_DYNAMIC_DRAW);
		glCheckError();
		glBufferSubData(GL_ARRAY_BUFFER, 0, Vertices.size() * sizeof(GLfloat), Vertices.data());
		glCheckError();

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, ShaderNameString + "Create vertex array object...");
		glGenVertexArrays(1, &VertexArrayObject);
		glCheckError();
		glBindVertexArray(VertexArrayObject);
		glCheckError();

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, ShaderNameString + "Create element buffer object...");
		GLuint ElementBufferObject{};
		glGenBuffers(1, &ElementBufferObject);
		glCheckError();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBufferObject);
		glCheckError();
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, Elements.size() * sizeof(GLint), Elements.data(), GL_STATIC_DRAW);
		glCheckError();

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, ShaderNameString + "Create and compile the vertex shader...");
		const std::string VertexShaderString{ LoadShaderSource(ShaderName + "Vert") };
		const GLchar* VertexShaderSource{ VertexShaderString.c_str() };
		const GLint VertexShaderSourceLength{ static_cast<GLint>(VertexShaderString.size()) };
		const GLuint VertexShader{ static_cast<GLuint>(glCreateShader(GL_VERTEX_SHADER)) };
		glCheckError();
		glShaderSource(VertexShader, 1, &VertexShaderSource, &VertexShaderSourceLength);
		glCheckError();
		glCompileShader(VertexShader);
		glCheckError();
		CheckCompileError(VertexShader, Components::Shader);

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, ShaderNameString + "Create and compile the fragment shader...");
		const std::string FragmentShaderString{ LoadShaderSource(ShaderName + "Frag") };
		const GLchar* FragmentShaderSource{ FragmentShaderString.c_str() };
		const GLint FragmentShaderSourceLength{ static_cast<GLint>(FragmentShaderString.size()) };
		const GLuint FragmentShader{ static_cast<GLuint>(glCreateShader(GL_FRAGMENT_SHADER)) };
		glCheckError();
		glShaderSource(FragmentShader, 1, &FragmentShaderSource, &FragmentShaderSourceLength);
		glCheckError();
		glCompileShader(FragmentShader);
		glCheckError();
		CheckCompileError(FragmentShader, Components::Shader);

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, ShaderNameString + "Link the vertex and fragment shader into a shader program...");
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

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, ShaderNameString + "Use shader program...");
		glUseProgram(ShaderProgram);
		glCheckError();

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, ShaderNameString + "Specify the layout of the vertex data...");
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

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, ShaderNameString + "Create projection matrix...");
		std::array<GLfloat, 16> ProjectionMatrix{};
		Ortho2D(ProjectionMatrix, 0.0F, static_cast<GLfloat>(Texture.Width), static_cast<GLfloat>(Texture.Height), 0.0F);
		glCheckError();
		const GLint Projection{ glGetUniformLocation(ShaderProgram, "MVP") };
		glCheckError();
		glUniformMatrix4fv(Projection, 1, GL_FALSE, ProjectionMatrix.data());
		glCheckError();

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, ShaderNameString + "Get opacity uniform location...");
		OpacityLocation = glGetUniformLocation(ShaderProgram, "Opacity");
		glCheckError();

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, ShaderNameString + "Since the shader program is now loaded into GPU, we can delete the shaders...");
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
		Blend ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
		glUseProgram(ShaderProgram);
		glUniform1f(OpacityLocation, Opacity);
		UpdateVertices(PosX, PosY, Width, Height);
		glBindTexture(GL_TEXTURE_2D, *TextureID);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	}

	inline void ShaderClass::LoadStaticTextureInGPU(const lwmf::TextureStruct& Texture, GLuint* TextureID, const std::int_fast32_t PosX, const std::int_fast32_t PosY, const std::int_fast32_t Width, const std::int_fast32_t Height)
	{
		UpdateVertices(PosX, PosY, Width, Height);
		LoadTextureInGPU(Texture, TextureID);
	}

	inline void ShaderClass::RenderStaticTexture(const GLuint* TextureID, const bool Blend, const float Opacity)
	{
		Blend ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
		glUseProgram(ShaderProgram);
		glUniform1f(OpacityLocation, Opacity);
		glBindVertexArray(VertexArrayObject);
		glBindTexture(GL_TEXTURE_2D, *TextureID);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	}

	inline void ShaderClass::PrepareLWMFTexture(const lwmf::TextureStruct& Texture, const std::int_fast32_t PosX, const std::int_fast32_t PosY)
	{
		UpdateVertices(PosX, PosY, Texture.Width, Texture.Height);
		glGenTextures(1, &OGLTextureID);
		glCheckError();
		glBindTexture(GL_TEXTURE_2D, OGLTextureID);
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
		Blend ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
		glUseProgram(ShaderProgram);
		glUniform1f(OpacityLocation, Opacity);
		glBindVertexArray(VertexArrayObject);
		glBindTexture(GL_TEXTURE_2D, OGLTextureID);
		FullscreenFlag ? glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Texture.Width, Texture.Height, GL_RGBA, GL_UNSIGNED_BYTE, Texture.Pixels.data()) : glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Texture.Width, Texture.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Texture.Pixels.data());
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	}

	inline void ShaderClass::Ortho2D(std::array<GLfloat, 16>& Matrix, const GLfloat Left, const GLfloat Right, const GLfloat Bottom, const GLfloat Top)
	{
		const GLfloat InvY{ 1.0F / (Top - Bottom) };
		const GLfloat InvX{ 1.0F / (Right - Left) };

		// First column
		Matrix[0] = 2.0F * InvX;

		// Second
		Matrix[5] = 2.0F * InvY;

		// Third
		Matrix[10] = -1.0F;

		// Fourth
		Matrix[12] = -(Right + Left) * InvX;
		Matrix[13] = -(Top + Bottom) * InvY;
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

	inline std::string ShaderClass::LoadShaderSource(const std::string& SourceName)
	{
		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, "Loading shader source: " + SourceName);

		std::string Result;

		if (SourceName == "DefaultFrag")
		{
			Result = DefaultFragmentShaderSource;
		}

		if (SourceName == "DefaultVert")
		{
			Result = DefaultVertexShaderSource;
		}

		return Result;
	}

	inline void ShaderClass::CheckError(const std::int_fast32_t Line)
	{
		// https://www.khronos.org/opengl/wiki/OpenGL_Error

		GLenum ErrorCode{};

		while ((ErrorCode = glGetError()) != GL_NO_ERROR)
		{
			std::map<GLenum, std::string> ErrorTable
			{
				{ GL_INVALID_ENUM, "An unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than to set the error flag." },
				{ GL_INVALID_VALUE, "A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag." },
				{ GL_INVALID_OPERATION, "The specified operation is not allowed in the current state. The offending command is ignored and has no other side effect than to set the error flag." },
				{ GL_STACK_OVERFLOW, "An attempt has been made to perform an operation that would cause an internal stack to overflow." },
				{ GL_STACK_UNDERFLOW, "An attempt has been made to perform an operation that would cause an internal stack to underflow." },
				{ GL_OUT_OF_MEMORY, "There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded." },
				{ GL_INVALID_FRAMEBUFFER_OPERATION, "The framebuffer object is not complete. The offending command is ignored and has no other side effect than to set the error flag." },
				{ GL_CONTEXT_LOST, "OpenGL context has been lost, due to a graphics card reset." }
			};

			const std::map<GLenum, std::string>::iterator ItErrorTable{ ErrorTable.find(ErrorCode) };

			ItErrorTable == ErrorTable.end() ? LWMFSystemLog.AddEntry(LogLevel::Critical, __FILENAME__, __LINE__, "Unknown OpenGL error in line " + std::to_string(Line) + "!") :
				LWMFSystemLog.AddEntry(LogLevel::Critical, __FILENAME__, __LINE__, "OpenGL error " + ItErrorTable->second + " in line " + std::to_string(Line) + "!");
		}
	}

	inline void ShaderClass::CheckCompileError(const GLuint Task, const Components Component)
	{
		GLint ErrorResult{};
		std::array<GLchar, 512> ErrorLog{};

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
					LWMFSystemLog.AddEntry(LogLevel::Critical, __FILENAME__, __LINE__, std::string(ErrorLog.data()));
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
					LWMFSystemLog.AddEntry(LogLevel::Critical, __FILENAME__, __LINE__, std::string(ErrorLog.data()));
				}

				break;
			}
			default: {}
		}
	}


} // namespace lwmf