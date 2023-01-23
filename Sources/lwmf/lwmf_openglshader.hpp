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
#include <string_view>
#include <array>
#include <map>

#include "lwmf_logging.hpp"
#include "lwmf_texture.hpp"

namespace lwmf
{


	//
	// Official OpenGL 4.5 Core Profile documentation:
	// https://registry.khronos.org/OpenGL/specs/gl/glspec45.core.pdf
	//
	// We are using DSA (Direct-State-Access)
	// https://www.khronos.org/opengl/wiki/Direct_State_Access
	//

	//
	// Shader source codes
	//

	inline constexpr std::string_view DefaultFragmentShaderSource
	{
		"#version 450 core\n"
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

	inline constexpr std::string_view DefaultVertexShaderSource
	{
		"#version 450 core\n"
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
		void RenderStaticTexture(const GLuint* TextureID, bool Blend, float Opacity) const;
		void PrepareLWMFTexture(const TextureStruct& Texture, std::int_fast32_t PosX, std::int_fast32_t PosY);
		void RenderLWMFTexture(const TextureStruct& Texture, bool Blend, float Opacity) const;

		GLuint OGLTextureID{};

	private:
		enum class Components : std::int_fast32_t
		{
			Shader,
			Program
		};

		static void Ortho2D(std::array<GLfloat, 16>& Matrix, GLfloat Left, GLfloat Right, GLfloat Bottom, GLfloat Top);
		void UpdateVertices(std::int_fast32_t PosX, std::int_fast32_t PosY, std::int_fast32_t Width, std::int_fast32_t Height);
		static std::string_view LoadShaderSource(std::string_view SourceName);
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
		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, ShaderNameString + "Start building shader...");

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, ShaderNameString + "Create vertex buffer object...");
		glCreateBuffers(1, &VertexBufferObject);
		glCheckError();
		glNamedBufferStorage(VertexBufferObject, Vertices.size() * sizeof(GLfloat), Vertices.data(), GL_DYNAMIC_STORAGE_BIT);
		glCheckError();

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, ShaderNameString + "Create indices buffer object...");

		GLuint IndicesBufferObject{};

		constexpr std::array<GLint, 6> Indices
		{
			0, 3, 2,
			2, 1, 0
		};

		glCreateBuffers(1, &IndicesBufferObject);
		glCheckError();
		glNamedBufferStorage(IndicesBufferObject, Indices.size() * sizeof(GLint), Indices.data(), GL_DYNAMIC_STORAGE_BIT);
		glCheckError();

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, ShaderNameString + "Create and compile the vertex shader...");
		const std::string VertexShaderString{ LoadShaderSource(ShaderName + "Vert") };
		const auto VertexShaderSource{ VertexShaderString.c_str() };
		const auto VertexShader{ glCreateShader(GL_VERTEX_SHADER) };
		glCheckError();
		glShaderSource(VertexShader, 1, &VertexShaderSource, nullptr);
		glCheckError();
		glCompileShader(VertexShader);
		glCheckError();
		CheckCompileError(VertexShader, Components::Shader);

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, ShaderNameString + "Create and compile the fragment shader...");
		const std::string FragmentShaderString{ LoadShaderSource(ShaderName + "Frag") };
		const auto FragmentShaderSource{ FragmentShaderString.c_str() };
		const auto FragmentShader{ glCreateShader(GL_FRAGMENT_SHADER) };
		glCheckError();
		glShaderSource(FragmentShader, 1, &FragmentShaderSource, nullptr);
		glCheckError();
		glCompileShader(FragmentShader);
		glCheckError();
		CheckCompileError(FragmentShader, Components::Shader);

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, ShaderNameString + "Link the vertex and fragment shader into a shader program...");
		ShaderProgram = glCreateProgram();
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

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, ShaderNameString + "Specify the layout of the vertex data...");
		glCreateVertexArrays(1, &VertexArrayObject);
		glCheckError();

		const auto PositionAttrib{ glGetAttribLocation(ShaderProgram, "position") };
		glCheckError();
		glEnableVertexArrayAttrib(VertexArrayObject, PositionAttrib);
		glCheckError();
		glVertexArrayAttribFormat(VertexArrayObject, PositionAttrib, 2, GL_FLOAT, GL_FALSE, 0);
		glCheckError();
		glVertexArrayAttribBinding(VertexArrayObject, PositionAttrib, 0);
		glCheckError();

		const auto TextureAttrib{ glGetAttribLocation(ShaderProgram, "texcoord") };
		glCheckError();
		glEnableVertexArrayAttrib(VertexArrayObject, TextureAttrib);
		glCheckError();
		glVertexArrayAttribFormat(VertexArrayObject, TextureAttrib, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat));
		glCheckError();
		glVertexArrayAttribBinding(VertexArrayObject, TextureAttrib, 0);
		glCheckError();

		glVertexArrayVertexBuffer(VertexArrayObject, 0, VertexBufferObject, 0, 4 * sizeof(GLfloat));
		glCheckError();
		glVertexArrayElementBuffer(VertexArrayObject, IndicesBufferObject);
		glCheckError();

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, ShaderNameString + "Create projection matrix...");
		std::array<GLfloat, 16> ProjectionMatrix{};
		Ortho2D(ProjectionMatrix, 0.0F, static_cast<GLfloat>(Texture.Width), static_cast<GLfloat>(Texture.Height), 0.0F);
		glProgramUniformMatrix4fv(ShaderProgram, glGetUniformLocation(ShaderProgram, "MVP"), 1, GL_FALSE, ProjectionMatrix.data());
		glCheckError();

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, ShaderNameString + "Get opacity uniform location...");
		OpacityLocation = glGetUniformLocation(ShaderProgram, "Opacity");
		glCheckError();
		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, ShaderNameString + "Opacity uniform location:" + std::to_string(OpacityLocation));

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
		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, ShaderNameString + "Setting flags...");
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glCheckError();
		glDisable(GL_DEPTH_TEST);
		glCheckError();
		glDisable(GL_DITHER);
		glCheckError();
		// Enable culling of back-facing facets
		// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glCullFace.xhtml
		glEnable(GL_CULL_FACE);
		glCheckError();
		glCullFace(GL_BACK);
		glCheckError();

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, ShaderNameString + "Finished building shader!");
	}

	inline void ShaderClass::LoadTextureInGPU(const lwmf::TextureStruct& Texture, GLuint *TextureID)
	{
		glCreateTextures(GL_TEXTURE_2D, 1, TextureID);
		glCheckError();
		glBindTexture(GL_TEXTURE_2D, *TextureID);
		glCheckError();
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Texture.Width, Texture.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Texture.Pixels.data());
		glCheckError();
		glTextureParameteri(*TextureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glCheckError();
		glTextureParameteri(*TextureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glCheckError();
	}

	inline void ShaderClass::RenderTexture(const GLuint* TextureID, const std::int_fast32_t PosX, const std::int_fast32_t PosY, const std::int_fast32_t Width, const std::int_fast32_t Height, const bool Blend, const float Opacity)
	{
		Blend ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
		UpdateVertices(PosX, PosY, Width, Height);
		glUseProgram(ShaderProgram);
		glProgramUniform1f(ShaderProgram, OpacityLocation, Opacity);
		glBindVertexArray(VertexArrayObject);
		glBindTexture(GL_TEXTURE_2D, *TextureID);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	}

	inline void ShaderClass::LoadStaticTextureInGPU(const lwmf::TextureStruct& Texture, GLuint* TextureID, const std::int_fast32_t PosX, const std::int_fast32_t PosY, const std::int_fast32_t Width, const std::int_fast32_t Height)
	{
		UpdateVertices(PosX, PosY, Width, Height);
		LoadTextureInGPU(Texture, TextureID);
	}

	inline void ShaderClass::RenderStaticTexture(const GLuint* TextureID, const bool Blend, const float Opacity) const
	{
		Blend ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
		glUseProgram(ShaderProgram);
		glProgramUniform1f(ShaderProgram, OpacityLocation, Opacity);
		glBindVertexArray(VertexArrayObject);
		glBindTexture(GL_TEXTURE_2D, *TextureID);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	}

	inline void ShaderClass::PrepareLWMFTexture(const lwmf::TextureStruct& Texture, const std::int_fast32_t PosX, const std::int_fast32_t PosY)
	{
		UpdateVertices(PosX, PosY, Texture.Width, Texture.Height);
		glCreateTextures(GL_TEXTURE_2D, 1, &OGLTextureID);
		glCheckError();
		glTextureStorage2D(OGLTextureID, 1, GL_RGBA8, Texture.Width, Texture.Height);
		glCheckError();
		glTextureParameteri(OGLTextureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glCheckError();
		glTextureParameteri(OGLTextureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glCheckError();
	}

	inline void ShaderClass::RenderLWMFTexture(const lwmf::TextureStruct& Texture, const bool Blend, const float Opacity) const
	{
		Blend ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
		glUseProgram(ShaderProgram);
		glProgramUniform1f(ShaderProgram, OpacityLocation, Opacity);
		glBindVertexArray(VertexArrayObject);
		glTextureSubImage2D(OGLTextureID, 0, 0, 0, Texture.Width, Texture.Height, GL_RGBA, GL_UNSIGNED_BYTE, Texture.Pixels.data());
		glBindTexture(GL_TEXTURE_2D, OGLTextureID);
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
		Vertices[0] = static_cast<GLfloat>(PosX);
		Vertices[1] = static_cast<GLfloat>(PosY);

		Vertices[4] = static_cast<GLfloat>(PosX + Width);
		Vertices[5] = static_cast<GLfloat>(PosY);

		Vertices[8] = static_cast<GLfloat>(PosX + Width);
		Vertices[9] = static_cast<GLfloat>(PosY + Height);

		Vertices[12] = static_cast<GLfloat>(PosX);
		Vertices[13] = static_cast<GLfloat>(PosY + Height);

		glNamedBufferSubData(VertexBufferObject, 0, Vertices.size() * sizeof(GLfloat), Vertices.data());
	}

	inline std::string_view ShaderClass::LoadShaderSource(const std::string_view SourceName)
	{
		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, __LINE__, "Loading shader source: " + std::string(SourceName));

		std::string_view Result;

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