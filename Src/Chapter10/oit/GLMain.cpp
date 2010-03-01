// GLMain.cpp
// Program entry point and main control is here.
/* 
Copyright (c) 2009, Richard S. Wright Jr.
GLTools Open Source Library
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list 
of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this list 
of conditions and the following disclaimer in the documentation and/or other 
materials provided with the distribution.

Neither the name of Richard S. Wright Jr. nor the names of other contributors may be used 
to endorse or promote products derived from this software without specific prior 
written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR 
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include "oit.h"
#include <gl\wglew.h>

// Our instance of the OpenGL renderer
OrderIndependentTransparancy	glRenderer;

//////////////////////////////////////////////////////////////////////////////
// Globals
// These may be shared across modules
HINSTANCE	ghInstance = NULL;		// Application instance
HWND		ghMainWnd = NULL;		// Main Window (window manager)
DEVMODE		gDevMode;

#ifdef UNICODE
const wchar_t szGameName[] = TEXT("Order Independent Transparancy");
#else
const char szGameName[] = "Order Independent Transparancy";
#endif

//////////////////////////////////////////////////////////////////////////////
// Forward  and external Declarations
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

///////////////////////////////////////////////////////////////////////////////
// Setup the actual window and related state.
// Create the window, find a pixel format, create the OpenGL context
int GetPF()
{
    bool bRetVal = true;

    int nWindowX = 0;
    int nWindowY = 0;
    int nWidth  = 10;
    int nHeight = 10;
    int nPixelFormat  = -1;
    PIXELFORMATDESCRIPTOR pfd;

    DWORD dwExtStyle;
    DWORD dwWindStyle;

    HWND         g_hWnd;
    HGLRC        g_hRC;
    HDC          g_hDC;
    WNDCLASS     g_windClass; 
    HINSTANCE g_hInstance = GetModuleHandle(NULL);
    RECT         g_windowRect;
    static const TCHAR g_szClassName[50]  =  TEXT("OGL_CLASS");
    static const TCHAR g_szAppName[] = TEXT("Oit");

    // setup window class
    g_windClass.lpszClassName = g_szClassName;                // Set the name of the Class
    g_windClass.lpfnWndProc   = (WNDPROC)MainWndProc;
    g_windClass.hInstance     = g_hInstance;                // Use this module for the module handle
    g_windClass.hCursor       = LoadCursor(NULL, IDC_ARROW);// Pick the default mouse cursor
    g_windClass.hIcon         = LoadIcon(NULL, IDI_WINLOGO);// Pick the default windows icons
    g_windClass.hbrBackground = NULL;                       // No Background
    g_windClass.lpszMenuName  = NULL;                       // No menu for this window
    g_windClass.style         = CS_HREDRAW | CS_OWNDC |     // set styles for this class, specifically to catch
                                CS_VREDRAW;                 // window redraws, unique DC, and resize
    g_windClass.cbClsExtra    = 0;                          // Extra class memory
    g_windClass.cbWndExtra    = 0;                          // Extra window memory

    // Register the newly defined class
    if(!RegisterClass( &g_windClass ))
        bRetVal = false;

    dwExtStyle  = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    dwWindStyle = WS_OVERLAPPEDWINDOW;
    ShowCursor(TRUE);

    g_windowRect.left   = nWindowX;
    g_windowRect.right  = nWindowX + nWidth;
    g_windowRect.top    = nWindowY;
    g_windowRect.bottom = nWindowY + nHeight;

    // Setup window width and height
    AdjustWindowRectEx(&g_windowRect, dwWindStyle, FALSE, dwExtStyle);

    //Adjust for adornments
    int nWindowWidth = g_windowRect.right   - g_windowRect.left;
    int nWindowHeight = g_windowRect.bottom - g_windowRect.top;

    // Create window
    g_hWnd = CreateWindowEx(dwExtStyle,     // Extended style
                            g_szClassName,    // class name
                            g_szAppName,   // window name
                            dwWindStyle |        
                            WS_CLIPSIBLINGS | 
                            WS_CLIPCHILDREN,// window stlye
                            nWindowX,       // window position, x
                            nWindowY,       // window position, y
                            nWindowWidth,   // height
                            nWindowHeight,  // width
                            NULL,           // Parent window
                            NULL,           // menu
                            g_hInstance,    // instance
                            NULL);          // pass this to WM_CREATE

    // now that we have a window, setup the pixel format descriptor
    g_hDC = GetDC(g_hWnd);

    // Set a dummy pixel format so that we can get access to wgl functions
    SetPixelFormat( g_hDC, 1,&pfd);
    // Create OGL context and make it current
    g_hRC = wglCreateContext( g_hDC );
    wglMakeCurrent( g_hDC, g_hRC );

    if (g_hDC == 0 ||
        g_hDC == 0)
    {
        bRetVal = false;
        printf("!!! An error occured creating an OpenGL window.\n");
    }

    // Setup GLEW which loads OGL function pointers
    GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
        bRetVal = false;
		printf("Error: %s\n", glewGetErrorString(err));
	}
    const GLubyte *oglVersion = glGetString(GL_VERSION);
    printf("This system supports OpenGL Version %s.\n", oglVersion);



    AdjustWindowRectEx(&g_windowRect, dwWindStyle, FALSE, dwExtStyle);

    // Create the window again
    g_hWnd = CreateWindowEx(dwExtStyle,     // Extended style
                            g_szClassName,    // class name
                            g_szAppName,   // window name
                            dwWindStyle |        
                            WS_CLIPSIBLINGS | 
                            WS_CLIPCHILDREN,// window stlye
                            nWindowX,       // window position, x
                            nWindowY,       // window position, y
                            nWindowWidth,   // height
                            nWindowHeight,  // width
                            NULL,           // Parent window
                            NULL,           // menu
                            g_hInstance,    // instance
                            NULL);          // pass this to WM_CREATE

    g_hDC = GetDC(g_hWnd);

    int nPixCount = 0;

    // Specify the important attributes we care about
    int pixAttribs[] = { WGL_SUPPORT_OPENGL_ARB, 1, // Must support OGL rendering
                         WGL_DRAW_TO_WINDOW_ARB, 1, // pf that can run a window
                         WGL_ACCELERATION_ARB,   1, // must be HW accelerated
                         WGL_COLOR_BITS_ARB,     24, // 8 bits of each R, G and B
                         WGL_DEPTH_BITS_ARB,     16, // 16 bits of depth precision for window
                         WGL_DOUBLE_BUFFER_ARB,	 GL_TRUE, // Double buffered context
		                 WGL_SAMPLE_BUFFERS_ARB, GL_TRUE, // MSAA on
		                 WGL_SAMPLES_ARB,        8, // 8x MSAA 
                         WGL_PIXEL_TYPE_ARB,      WGL_TYPE_RGBA_ARB, // pf should be RGBA type
                         0}; // NULL termination

    // Ask OpenGL to find the most relevant format matching our attribs
    // Only get one format back.
    wglChoosePixelFormatARB(g_hDC, &pixAttribs[0], NULL, 1, &nPixelFormat, (UINT*)&nPixCount);

    if(nPixelFormat == -1) 
    {
        printf("!!! An error occurred trying to find a MSAA pixel format with the requested attribs.\n");

        // Try again without MSAA
        pixAttribs[15] = 1;
        wglChoosePixelFormatARB(g_hDC, &pixAttribs[0], NULL, 1, &nPixelFormat, (UINT*)&nPixCount);
            
        if(nPixelFormat == -1) 
        {
            // Couldn't find a format, perhaps no 3D HW or drivers are installed
            g_hDC = 0;
            g_hDC = 0;
            bRetVal = false;
            printf("!!! An error occurred trying to find a pixel format with the requested attribs.\n");
        }
    }
    
    if(nPixelFormat != -1) 
    {
        // Check for MSAA
        int attrib[] = { WGL_SAMPLES_ARB };
        int nResults = 0;
        wglGetPixelFormatAttribivARB(g_hDC, nPixelFormat, 0, 1, attrib, &nResults);
        printf("Chosen pixel format is MSAA with %d samples.\n", nResults);
    }
    // Now that extensions are setup, delete window and start over picking a real format.
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(g_hRC);
    ReleaseDC(g_hWnd, g_hDC);
    DestroyWindow(g_hWnd);
    
    return nPixelFormat;
}


//////////////////////////////////////////////////////////////////////////////
// Program Entry Point
//////////////////////////////////////////////////////////////////////////////
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
	{
	/////////////////////////////////////////////
	// Do any program wide Initialization here
	// Change display settings 
	if(glRenderer.GetFullScreen() == GL_TRUE)
		{
		gDevMode.dmPelsHeight = glRenderer.GetHeight();
		gDevMode.dmPelsWidth = glRenderer.GetWidth();
		gDevMode.dmSize = sizeof(DEVMODE);
		gDevMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
		if(ChangeDisplaySettings(&gDevMode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
			{
			MessageBox(NULL, TEXT("Cannot change to selected desktop resolution."),
							  NULL, MB_OK | MB_ICONSTOP);
			return -1;
			}
		}

	/////////////////////////////////////////////
	// Create Main Window. 
	WNDCLASSEX wcex;
	wcex.cbSize			= sizeof(WNDCLASSEX); 
	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc	= (WNDPROC)MainWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= (HCURSOR)LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL; //(HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("OpenGLWin32Window"); // Should be unique
	wcex.hIconSm		= NULL;

	if(RegisterClassEx(&wcex) == 0)
		return -1;

	// Select window styles
	UINT uiStyle,uiStyleX;
	if(glRenderer.GetFullScreen() == GL_TRUE)
		{
		uiStyle = WS_POPUP;
		uiStyleX = WS_EX_TOPMOST;
		}
	else
		{
		uiStyle = WS_OVERLAPPEDWINDOW;
		uiStyleX = NULL;
		}

	// Create the main 3D window
	ghMainWnd = CreateWindowEx(uiStyleX, wcex.lpszClassName, szGameName, uiStyle,
      0, 0, glRenderer.GetWidth(), glRenderer.GetHeight(), NULL, NULL, hInstance, NULL);

	if (!ghMainWnd)
		return -1;

	// Make sure window manager stays hidden
	ShowWindow(ghMainWnd, SW_SHOW);
	UpdateWindow(ghMainWnd);
	//SetFocus(ghMainWnd);

	/////////////////////////////////////////////
	// Message Pump - Use the form that goes idle and waits for 
	// messages, not continually running.
	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0)) 
		{
		if(!TranslateMessage(&msg)) 
			{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			}
		}


	/////////////////////////////////////////////
	// Do any program wide shutdown here
	// Restore Display Settings
	if(glRenderer.GetFullScreen() == GL_TRUE)
		ChangeDisplaySettings(NULL, 0);

	// Return termination code
	return msg.wParam;
	}


/////////////////////////////////////////////////////////////////////
// Main window message procedure. This is the window manager for
// the application
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, 
								WPARAM wParam, LPARAM lParam)
	{
	static HDC	hDC = NULL;		// GDI Device context
	static HGLRC hRC = NULL;	// OpenGL Rendering context

	switch (message) 
		{
		// Window is created, the application is starting
		case WM_CREATE:
			{
                int nPF = 0;
			hDC = GetDC(hWnd);
            PIXELFORMATDESCRIPTOR pfd = {
				sizeof(PIXELFORMATDESCRIPTOR),		// Size of struct
				1,									// Version of struct
				PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, // 
				PFD_TYPE_RGBA,	// Always put this
				32,				// Always put this, size of the color buffer
				0,0,0,0,0,0,0,0,0,0,0,0,0, // You have no control over
				16,					       // Depth of the Z buffer
				8,				// Stencil buffer
				0,0,0,0,0,0 };	// Either obsolete, or we don't care about

                static int old = 0;
                static int firstCall = 0;
                if (firstCall != 0)
                    return 0;

                if (old == 0 && firstCall == 0)
                {
                    firstCall = 1;
                    nPF = 23; //GetPF();
                }
                else
                    nPF = ChoosePixelFormat(hDC, &pfd);

			SetPixelFormat(hDC, nPF, &pfd);

			// Create rendering context
			hRC = wglCreateContext(hDC);
			wglMakeCurrent(hDC, hRC);
			glRenderer.Initialize();
			}
			break;

		// Do not validate, this is sent repeatedly
		case WM_PAINT:
			glRenderer.Render();
			SwapBuffers(hDC);
			if(!glRenderer.GetAnimated())
				ValidateRect(hWnd, NULL);
			break;

		case WM_SIZE:
			glRenderer.Resize(LOWORD(lParam), HIWORD(lParam));
			break;

		// Window is destroyed, it's the end of the application
		case WM_DESTROY:
			// Cleanup
			glRenderer.Shutdown();
			wglMakeCurrent(hDC, NULL);
			wglDeleteContext(hRC);
			ReleaseDC(hWnd, hDC);
			PostQuitMessage(0);
			break;

		// Window is either full screen, or not visible
		case WM_ACTIVATE:
			{
			// Only handled when full screen mode
			if(glRenderer.GetFullScreen())
				{
				WINDOWPLACEMENT wndPlacement;
				wndPlacement.length = sizeof(WINDOWPLACEMENT);
				wndPlacement.flags = WPF_RESTORETOMAXIMIZED;
				wndPlacement.ptMaxPosition.x = 0;
				wndPlacement.ptMaxPosition.y = 0;
				wndPlacement.ptMinPosition.x = 0;
				wndPlacement.ptMinPosition.y = 0;
				wndPlacement.rcNormalPosition.bottom = gDevMode.dmPelsHeight;
				wndPlacement.rcNormalPosition.left = 0;
				wndPlacement.rcNormalPosition.top = 0;
				wndPlacement.rcNormalPosition.right = gDevMode.dmPelsWidth;

				if(LOWORD(wParam) == WA_INACTIVE)
					{
					wndPlacement.showCmd = SW_SHOWMINNOACTIVE;
					SetWindowPlacement(hWnd, &wndPlacement);
					ShowCursor(TRUE);
					}
				else
					{
					wndPlacement.showCmd = SW_RESTORE;
					SetWindowPlacement(hWnd, &wndPlacement);
					ShowCursor(FALSE);
					}
				}
			}
			break;

	
		// Handle keyboard input
		case WM_CHAR:
			// Close program on ESC key press
			if(wParam == 27)
				DestroyWindow(hWnd);
			break;

		case WM_CLOSE:
			DestroyWindow(hWnd);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	return 0;
	}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

