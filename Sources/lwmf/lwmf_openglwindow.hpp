/*
****************************************************
*                                                  *
* lwmf_openglwindow - lightweight media framework  *
*                                                  *
* (C) 2019 - present by Stefan Kubsch              *
*                                                  *
****************************************************
*/

#pragma once

#include <Windows.h>
#include <cstdint>

#include "lwmf_logging.hpp"
#include "lwmf_texture.hpp"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

namespace lwmf
{


	void ResizeViewportAndRenderTarget(TextureStruct& Texture, std::int_fast32_t Width, std::int_fast32_t Height);
	void CreateOpenGLWindow(HINSTANCE hInstance, TextureStruct& RenderTarget, std::int_fast32_t Width, std::int_fast32_t Height, LPCSTR WindowName, bool Fullscreen);
	void ResizeOpenGLWindow(TextureStruct& RenderTarget);
	void ClearBuffer();
	void SwapBuffer();
	void DeleteOpenGLContext();

	//
	// Variables and constants
	//

	inline HDC WindowHandle{};
	inline HWND MainWindow{};

	//
	// Functions
	//

	inline void ResizeViewportAndRenderTarget(TextureStruct& Texture, const std::int_fast32_t Width, const std::int_fast32_t Height)
	{
		CreateTexture(Texture, Width, Height, 0x00000000);
		glViewport(0, 0, Texture.Width, Texture.Height);
	}

	inline void CreateOpenGLWindow(const HINSTANCE hInstance, TextureStruct& RenderTarget, const std::int_fast32_t Width, const std::int_fast32_t Height, const LPCSTR WindowName, const bool Fullscreen)
	{
		// Create window

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "Create window...");

		if (Width <= 0 || Height <= 0)
		{
			LWMFSystemLog.AddEntry(LogLevel::Critical, __FILENAME__, "Value for window width or height is zero or negative! Check your parameters in lwmf::CreateOpenGLWindow()!");
		}

		WNDCLASS WindowClass{};
		WindowClass.lpfnWndProc = WndProc;
		WindowClass.hInstance = hInstance;
		WindowClass.lpszClassName = "lwmf";

		if (RegisterClass(&WindowClass) == 0)
		{
			LWMFSystemLog.AddEntry(LogLevel::Critical, __FILENAME__, "Error registering windowclass (RegisterClass)!");
		}
		else
		{
			DWORD dwExStyle{ WS_EX_APPWINDOW | WS_EX_WINDOWEDGE };
			DWORD dwStyle{ WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN };

			if (Fullscreen)
			{
				DEVMODE ScreenSettings{};
				ScreenSettings.dmSize = sizeof(DEVMODE);
				ScreenSettings.dmPelsWidth = static_cast<DWORD>(Width);
				ScreenSettings.dmPelsHeight = static_cast<DWORD>(Height);
				ScreenSettings.dmBitsPerPel = 32;
				ScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

				if (ChangeDisplaySettings(&ScreenSettings, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL)
				{
					dwExStyle = WS_EX_APPWINDOW;
					dwStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
					ShowCursor(FALSE);

					FullscreenFlag = true;
				}
			}

			RECT WinRect{ 0, 0, Width, Height };
			AdjustWindowRectEx(&WinRect, dwStyle, FALSE, dwExStyle);
			MainWindow = CreateWindowEx(dwExStyle, WindowClass.lpszClassName, WindowName, dwStyle, 0, 0, WinRect.right - WinRect.left, WinRect.bottom - WinRect.top, nullptr, nullptr, hInstance, nullptr);

			if (MainWindow == nullptr)
			{
				LWMFSystemLog.AddEntry(LogLevel::Critical, __FILENAME__, "Error creating window (CreateWindowEx)!");
			}

			// Create OpenGL context
			// https://www.khronos.org/opengl/wiki/Creating_an_OpenGL_Context_(WGL)

			LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "Create OpenGL context...");

			const PIXELFORMATDESCRIPTOR PFD
			{
				sizeof(PIXELFORMATDESCRIPTOR),
				1,
				PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
				PFD_TYPE_RGBA,
				32,
				0, 0, 0, 0, 0, 0,
				0,
				0,
				0,
				0, 0, 0, 0,
				24,
				8,
				0,
				PFD_MAIN_PLANE,
				0,
				0, 0, 0
			};

			WindowHandle = GetDC(MainWindow);

			if (WindowHandle == nullptr)
			{
				LWMFSystemLog.AddEntry(LogLevel::Critical, __FILENAME__, "Error creating WindowHandle (GetDC)!");
			}
			else
			{
				if (SetPixelFormat(WindowHandle, ChoosePixelFormat(WindowHandle, &PFD), &PFD) == 0)
				{
					LWMFSystemLog.AddEntry(LogLevel::Critical, __FILENAME__, "Error setting pixel format (SetPixelFormat)!");
				}
				else
				{
					if (wglMakeCurrent(WindowHandle, wglCreateContext(WindowHandle)) == 0)
					{
						LWMFSystemLog.AddEntry(LogLevel::Critical, __FILENAME__, "Error creating OpenGL context (wglMakeCurrent)!");
					}
					else
					{
						ShowWindow(MainWindow, SW_SHOW);
						SetForegroundWindow(MainWindow);
						SetFocus(MainWindow);
						ResizeViewportAndRenderTarget(RenderTarget, Width, Height);
					}
				}

				// Get OpenGL system information

				LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "Gather OpenGL system information...");

				GLint MajorVersion{};
				GLint MinorVersion{};

				glGetIntegerv(GL_MAJOR_VERSION, &MajorVersion);
				glGetIntegerv(GL_MINOR_VERSION, &MinorVersion);

				LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "OpenGL version: " + std::to_string(MajorVersion) + "." + std::to_string(MinorVersion));
				LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "OpenGL vendor: " + std::string(reinterpret_cast<const char*>(glGetString(GL_VENDOR))));
				LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "OpenGL renderer: " + std::string(reinterpret_cast<const char*>(glGetString(GL_RENDERER))));
				LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "Primary OpenGL shading language version: " + std::string(reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION))));
			}
		}
	}

	inline void ResizeOpenGLWindow(TextureStruct& RenderTarget)
	{
		RECT WinRect{};
		GetClientRect(MainWindow, &WinRect);
		ResizeViewportAndRenderTarget(RenderTarget, WinRect.right, WinRect.bottom);
	}

	inline void ClearBuffer()
	{
		glColor4f(0.0F, 0.0F, 0.0F, 0.0F);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	inline void SwapBuffer()
	{
		SwapBuffers(WindowHandle);
	}

	inline void DeleteOpenGLContext()
	{
		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "Delete OpenGL context...");

		const HGLRC OpenGLContext{ wglGetCurrentContext() };

		if (OpenGLContext != nullptr)
		{
			wglMakeCurrent(nullptr, nullptr);

			if (ReleaseDC(MainWindow, WindowHandle) == 1)
			{
				wglDeleteContext(OpenGLContext);
			}
			else
			{
				LWMFSystemLog.AddEntry(LogLevel::Warn, __FILENAME__, "Error deleting OpenGL context (ReleaseDC)!");
			}
		}
	}


} // namespace lwmf