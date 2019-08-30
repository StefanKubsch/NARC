/*
****************************************************
*                                                  *
* lwmf_openglloader - lightweight media framework  *
*                                                  *
* (C) 2019 - present by Stefan Kubsch              *
*                                                  *
****************************************************
*/

#pragma once

#include <Windows.h>
#include <cstddef>
#include <GL/gl.h>

#include "lwm_logging.hpp"

#pragma comment (lib, "opengl32.lib")

#define GL_ARRAY_BUFFER						0x8892
#define GL_COMPILE_STATUS					0x8B81
#define GL_DYNAMIC_DRAW						0x88E8
#define GL_ELEMENT_ARRAY_BUFFER				0x8893
#define GL_FRAGMENT_SHADER					0x8B30
#define GL_INVALID_FRAMEBUFFER_OPERATION	0x0506
#define GL_LINK_STATUS						0x8B82
#define GL_STATIC_DRAW						0x88E4
#define GL_TEXTURE0							0x84C0
#define GL_VERTEX_SHADER					0x8B31

using GLchar = char;
using GLsizeiptr = std::ptrdiff_t;
using GLintptr = std::ptrdiff_t;

#define OGL \
	OG(void,	glAttachShader,				GLuint program, GLuint shader) \
	OG(void,	glBindBuffer,				GLenum target, GLuint buffer) \
	OG(void,	glBindFragDataLocation,		GLuint program, GLuint colorNumber,	const char* name) \
	OG(void,	glBindVertexArray,			GLuint array) \
	OG(void,	glBufferData,				GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage) \
	OG(void,	glBufferSubData,			GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data) \
	OG(void,	glCompileShader,			GLuint shader) \
	OG(GLint,	glCreateProgram,			void) \
	OG(GLint,	glCreateShader,				GLenum type) \
	OG(void,	glDeleteProgram,			GLuint program) \
	OG(void,	glDeleteShader,				GLuint shader) \
	OG(void,	glDetachShader,				GLuint program, GLuint shader) \
	OG(void,	glEnableVertexAttribArray,	GLuint index) \
	OG(void,	glGenBuffers,				GLsizei n, GLuint* buffers) \
	OG(void,	glGenVertexArrays,			GLsizei n, GLuint* arrays) \
	OG(GLint,	glGetAttribLocation,		GLuint program, const GLchar *name) \
	OG(void,	glGetProgramInfoLog,		GLuint program, GLsizei maxLength, GLsizei* length, GLchar* infoLog) \
	OG(void,	glGetProgramiv,				GLuint program, GLenum pname, GLint* params) \
	OG(void,	glGetShaderInfoLog,			GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog) \
	OG(void,	glGetShaderiv,				GLuint shader, GLenum pname, GLint* params) \
	OG(GLint,	glGetUniformLocation,		GLuint program, const GLchar* name) \
	OG(void,	glLinkProgram,				GLuint program) \
	OG(void,	glShaderSource,				GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length) \
	OG(void,	glTexStorage2D,				GLenum target,GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) \
	OG(void,	glUniform1f,				GLint location, GLfloat v0) \
	OG(void,	glUniformMatrix4fv,			GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) \
	OG(void,	glUseProgram,				GLuint program) \
	OG(void,	glVertexAttribPointer,		GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer)

#define OG(Return, Name, ...) typedef Return WINAPI Name##proc(__VA_ARGS__); extern Name##proc* Name;
	OGL
#undef OG

#define OG(Return, Name, ...) Name##proc * Name;
	OGL
#undef OG

namespace lwmf
{


	inline void InitOpenGLLoader()
	{
		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "Load wgl extensions...");
		const HMODULE OGL32{ LoadLibrary("opengl32.dll") };

		if (OGL32 == nullptr)
		{
			LWMFSystemLog.AddEntry(LogLevel::Critical, __FILENAME__, "Error loading opengl32.dll!");
		}
		else
		{
			using wglGetProcAddressproc = PROC WINAPI(LPCSTR);
			wglGetProcAddressproc* wglGetProcAddress{ reinterpret_cast<wglGetProcAddressproc*>(GetProcAddress(OGL32, "wglGetProcAddress")) };

			#define OG(Return, Name, ...) Name = (Name##proc *)wglGetProcAddress(#Name);
				OGL
			#undef OG

			FreeLibrary(OGL32);
		}
	}

	inline void SetVSync(const std::int_fast32_t Sync)
	{
		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "Set vsync (" + std::to_string(Sync) + ")...");
		using PFNWGLSWAPINTERVALFARPROC = PROC WINAPI(std::int_fast32_t);
		PFNWGLSWAPINTERVALFARPROC* wglSwapIntervalEXT{ reinterpret_cast<PFNWGLSWAPINTERVALFARPROC*>(wglGetProcAddress("wglSwapIntervalEXT")) };
		wglSwapIntervalEXT(Sync);
	}


} // namespace lwmf