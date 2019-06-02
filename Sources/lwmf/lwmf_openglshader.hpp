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
#include <string>
#include <fstream>
#include <limits>
#include <sstream>

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

		// Create buffers
		glGenBuffers(1, &VertexBufferObject);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);
		glBufferData(GL_ARRAY_BUFFER, 2048, nullptr, GL_DYNAMIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);

		glGenVertexArrays(1, &VertexArrayObject);
		glBindVertexArray(VertexArrayObject);

		glGenBuffers(1, &ElementBufferObject);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBufferObject);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Elements), Elements, GL_STATIC_DRAW);

		// Create and compile the vertex shader
		const GLchar* VertexShaderSource{ LoadShaderSource("./Shader/Vertex/" + ShaderName + ".vert") };
		const GLint VertexShader{ glCreateShader(GL_VERTEX_SHADER) };
		glShaderSource(VertexShader, 1, &VertexShaderSource, nullptr);
		glCompileShader(VertexShader);
		CheckCompileError(VertexShader, Components::Shader);

		// Create and compile the fragment shader
		const GLchar* FragmentShaderSource{ LoadShaderSource("./Shader/Fragment/" + ShaderName + ".frag") };
		const GLint FragmentShader{ glCreateShader(GL_FRAGMENT_SHADER) };
		glShaderSource(FragmentShader, 1, &FragmentShaderSource, nullptr);
		glCompileShader(FragmentShader);
		CheckCompileError(FragmentShader, Components::Shader);

		// Link the vertex and fragment shader into a shader program
		const GLint ShaderProgram{ glCreateProgram() };
		glAttachShader(ShaderProgram, VertexShader);
		glAttachShader(ShaderProgram, FragmentShader);
		glBindFragDataLocation(ShaderProgram, 0, "outColor");
		glLinkProgram(ShaderProgram);

		// Check shader program
		CheckCompileError(ShaderProgram, Components::Program);

		// Everything´s fine, now we can use the shader program...
		glUseProgram(ShaderProgram);

		// Specify the layout of the vertex data
		const GLint PositionAttrib{ glGetAttribLocation(ShaderProgram, "position") };
		glEnableVertexAttribArray(PositionAttrib);
		glVertexAttribPointer(PositionAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);

		const GLint TextureAttrib{ glGetAttribLocation(ShaderProgram, "texcoord") };
		glEnableVertexAttribArray(TextureAttrib);
		glVertexAttribPointer(TextureAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), reinterpret_cast<GLvoid*>(2 * sizeof(GLfloat))); //-V566

		float ProjectionMatrix[16];
		Ortho2D(ProjectionMatrix, 0.0F, static_cast<GLfloat>(Texture.Width), static_cast<GLfloat>(Texture.Height), 0.0F);
		const GLint Projection{ glGetUniformLocation(ShaderProgram, "MVP") };
		glUniformMatrix4fv(Projection, 1, GL_FALSE, ProjectionMatrix);

		// Since the shader program is now loaded into GPU, we can delete the shader program...
		glDetachShader(ShaderProgram, FragmentShader);
		glDetachShader(ShaderProgram, VertexShader);
		glDeleteShader(FragmentShader);
		glDeleteShader(VertexShader);
	}

	inline void ShaderClass::LoadTextureInGPU(const lwmf::TextureStruct& Texture, GLuint* TextureID)
	{
		glGenTextures(1, TextureID);
		glBindTexture(GL_TEXTURE_2D, *TextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Texture.Width, Texture.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Texture.Pixels.data());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
		glBindTexture(GL_TEXTURE_2D, LWMFTextureID);

		if (FullscreenFlag == 1)
		{
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, Texture.Width, Texture.Height);
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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

	inline void ShaderClass::CheckCompileError(const GLint Task, const Components Component)
	{
		GLint ErrorResult{};

		switch (Component)
		{
			case Components::Shader:
			{
				glGetShaderiv(Task, GL_COMPILE_STATUS, &ErrorResult);

				if (ErrorResult == GL_FALSE)
				{
					exit(-1);
				}
				break;
			}
			case Components::Program:
			{
				glGetProgramiv(Task, GL_LINK_STATUS, &ErrorResult);

				if (ErrorResult == GL_FALSE)
				{
					exit(-1);
				}
				break;
			}
			default: {}
		}
	}


} // namespace lwmf