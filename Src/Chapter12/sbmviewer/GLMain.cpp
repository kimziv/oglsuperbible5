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
#include "sbmviewer.h"

// Our instance of the OpenGL renderer
SBMViewer    glRenderer;

//////////////////////////////////////////////////////////////////////////////
// Globals
// These may be shared across modules
HINSTANCE    ghInstance = NULL;        // Application instance
HWND        ghMainWnd = NULL;        // Main Window (window manager)
DEVMODE        gDevMode;

#ifdef UNICODE
const wchar_t szGameName[] = TEXT("Instancing");
#else
const char szGameName[] = "Instancing";
#endif

//////////////////////////////////////////////////////////////////////////////
// Forward  and external Declarations
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


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
    wcex.cbSize            = sizeof(WNDCLASSEX); 
    wcex.style            = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wcex.lpfnWndProc    = (WNDPROC)MainWndProc;
    wcex.cbClsExtra        = 0;
    wcex.cbWndExtra        = 0;
    wcex.hInstance        = hInstance;
    wcex.hIcon            = NULL;
    wcex.hCursor        = (HCURSOR)LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground    = NULL; //(HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName    = NULL;
    wcex.lpszClassName    = TEXT("OpenGLWin32Window"); // Should be unique
    wcex.hIconSm        = NULL;

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
    static HDC    hDC = NULL;        // GDI Device context
    static HGLRC hRC = NULL;    // OpenGL Rendering context

    switch (message) 
        {
        // Window is created, the application is starting
        case WM_CREATE:
            {
            hDC = GetDC(hWnd);

            static PIXELFORMATDESCRIPTOR pfd = {
                sizeof(PIXELFORMATDESCRIPTOR),        // Size of struct
                1,                                    // Version of struct
                PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, // 
                PFD_TYPE_RGBA,    // Always put this
                32,                // Always put this, size of the color buffer
                0,0,0,0,0,0,0,0,0,0,0,0,0, // You have no control over
                16,                           // Depth of the Z buffer
                8,                // Stencil buffer
                0,0,0,0,0,0 };    // Either obsolete, or we don't care about

            int nPF = ChoosePixelFormat(hDC, &pfd);

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

