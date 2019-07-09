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
#include <vector>

#include "lwm_logging.hpp"
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

	inline HDC WindowHandle;
	inline HWND MainWindow;

	//
	// Functions
	//

	inline void ResizeViewportAndRenderTarget(TextureStruct& Texture, const std::int_fast32_t Width, const std::int_fast32_t Height)
	{
		SetTextureMetrics(Texture, Width, Height);
		glViewport(0, 0, Texture.Width, Texture.Height);

		Texture.Pixels.clear();
		Texture.Pixels.shrink_to_fit();
		Texture.Pixels.resize(static_cast<size_t>(Texture.Size), 0);
	}

	inline void CreateOpenGLWindow(const HINSTANCE hInstance, TextureStruct& RenderTarget, const std::int_fast32_t Width, const std::int_fast32_t Height, const LPCSTR WindowName, const bool Fullscreen)
	{
		// Create window

		LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "Create window...");
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
				DEVMODE ScreenSettings;
				std::memset(&ScreenSettings, 0, sizeof(ScreenSettings));
				ScreenSettings.dmSize = sizeof(ScreenSettings);
				ScreenSettings.dmPelsWidth = Width;
				ScreenSettings.dmPelsHeight = Height;
				ScreenSettings.dmBitsPerPel = 32;
				ScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

				if (ChangeDisplaySettings(&ScreenSettings, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL)
				{
					dwExStyle = WS_EX_APPWINDOW;
					dwStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
					ShowCursor(FALSE);

					FullscreenFlag = 1;
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

			LWMFSystemLog.AddEntry(LogLevel::Info, __FILENAME__, "Create OpenGL context...");

			const PIXELFORMATDESCRIPTOR PFD =
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
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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