// SphereWorld_redux.cpp
// OpenGL SuperBible Chapter 13
// New and improved (performance) sphere world
// Program by Richard S. Wright Jr. and Nicholas Haemel

#include <gltools.h>
#include <GLShaderManager.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <Stopwatch.h>
#include <gl\wglew.h>
#include <math.h>
#include <stdio.h>
#include <iostream>


GLShaderManager		shaderManager;			// Shader Manager
GLMatrixStack		modelViewMatrix;		// Modelview Matrix
GLMatrixStack		projectionMatrix;		// Projection Matrix
GLFrustum			viewFrustum;			// View Frustum
GLGeometryTransform	transformPipeline;		// Geometry Transform Pipeline
GLFrame				cameraFrame;			// Camera frame

GLTriangleBatch		torusBatch;
GLTriangleBatch		sphereBatch;
GLBatch				groundBatch;
GLBatch				floorBatch;

GLuint				uiTextures[3];

static const TCHAR g_szAppName[] = TEXT("Sphere World Redux");
static const TCHAR g_szClassName[50]  =  TEXT("OGL_CLASS");

static const int g_nWinWidth  = 800;
static const int g_nWinHeight = 600;

    HWND         g_hWnd;
    HGLRC        g_hRC;
    HDC          g_hDC;
HINSTANCE    g_hInstance;
WNDCLASS     g_windClass; 
RECT         g_windowRect;
bool         g_ContinueRendering;
bool         g_InFullScreen;

void DrawSongAndDance(GLfloat yRot)		// Called to draw dancing objects
{
	static GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	static GLfloat vLightPos[] = { 0.0f, 3.0f, 0.0f, 1.0f };
	
	// Get the light position in eye space
	M3DVector4f	vLightTransformed;
	M3DMatrix44f mCamera;
	modelViewMatrix.GetMatrix(mCamera);
	m3dTransformVector4(vLightTransformed, vLightPos, mCamera);
	
	// Draw the light source
	modelViewMatrix.PushMatrix();
	modelViewMatrix.Translatev(vLightPos);
	shaderManager.UseStockShader(GLT_SHADER_FLAT, 
								 transformPipeline.GetModelViewProjectionMatrix(),
								 vWhite);
	sphereBatch.Draw();
	modelViewMatrix.PopMatrix();
	
	// Song and dance
	modelViewMatrix.Translate(0.0f, 0.2f, -2.5f);
	modelViewMatrix.PushMatrix();	// Saves the translated origin
	modelViewMatrix.Rotate(yRot, 0.0f, 1.0f, 0.0f);
	
	// Draw stuff relative to the camera
	glBindTexture(GL_TEXTURE_2D, uiTextures[1]);
	shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
								 modelViewMatrix.GetMatrix(),
								 transformPipeline.GetProjectionMatrix(),
								 vLightTransformed, 
								 vWhite,
								 0);
	torusBatch.Draw();
	modelViewMatrix.PopMatrix(); // Erased the rotate
	
	modelViewMatrix.Rotate(yRot * -2.0f, 0.0f, 1.0f, 0.0f);
	modelViewMatrix.Translate(0.8f, 0.0f, 0.0f);
	
	glBindTexture(GL_TEXTURE_2D, uiTextures[2]);
	shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
								 modelViewMatrix.GetMatrix(),
								 transformPipeline.GetProjectionMatrix(),
								 vLightTransformed, 
								 vWhite,
								 0);
	sphereBatch.Draw();
}
	
	
bool LoadTGATexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
{
	GLbyte *pBits;
	int nWidth, nHeight, nComponents;
	GLenum eFormat;
	
	// Read the texture bits
	pBits = gltReadTGABits(szFileName, &nWidth, &nHeight, &nComponents, &eFormat);
	if(pBits == NULL) 
		return false;
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
	
	if(minFilter == GL_LINEAR_MIPMAP_LINEAR || 
	   minFilter == GL_LINEAR_MIPMAP_NEAREST ||
	   minFilter == GL_NEAREST_MIPMAP_LINEAR ||
	   minFilter == GL_NEAREST_MIPMAP_NEAREST)
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, nComponents, nWidth, nHeight, 0,
				 eFormat, GL_UNSIGNED_BYTE, pBits);
	
	free(pBits);
	return true;
}

        
//////////////////////////////////////////////////////////////////
// This function does any needed initialization on the rendering
// context. 
void SetupRC()
{
	// Initialze Shader Manager
	shaderManager.InitializeStockShaders();
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
	// This makes a torus
	gltMakeTorus(torusBatch, 0.4f, 0.15f, 20, 20);
	
	// This makes a sphere
	gltMakeSphere(sphereBatch, 0.1f, 26, 13);
	
	// This makes the ground lines
	groundBatch.Begin(GL_LINES, 328);
	for(GLfloat x = -20.0f; x <= 20.0f; x+= 0.5f)
	{
		groundBatch.Vertex3f(x, -0.4f, 20.0f);	// Parallel to X, cross the z axis
		groundBatch.Vertex3f(x, -0.4f, -20.0f);
		
		groundBatch.Vertex3f(20.0f, -0.4f, x); // Parallel to Z, cross the x axis
		groundBatch.Vertex3f(-20.0f, -0.4f, x);
	}
	groundBatch.End();
	
	
	// Make the solid ground
	GLfloat alpha = 0.25f;
	GLfloat texSize = 10.0f;
	floorBatch.Begin(GL_TRIANGLE_FAN, 4, 1);
	floorBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	floorBatch.Color4f(1.0f, 1.0f, 1.0f, alpha);
	floorBatch.Vertex3f(-20.0f, -0.41f, 20.0f);
	
	floorBatch.MultiTexCoord2f(0, texSize, 0.0f);
	floorBatch.Color4f(1.0f, 1.0f, 1.0f, alpha);
	floorBatch.Vertex3f(20.0f, -0.41f, 20.0f);
	
	floorBatch.MultiTexCoord2f(0, texSize, texSize);
	floorBatch.Color4f(1.0f, 1.0f, 1.0f, alpha);
	floorBatch.Vertex3f(20.0f, -0.41f, -20.0f);
	
	floorBatch.MultiTexCoord2f(0, 0.0f, texSize);
	floorBatch.Color4f(1.0f, 1.0f, 1.0f, alpha);
	floorBatch.Vertex3f(-20.0f, -0.41f, -20.0f);
	floorBatch.End();
	
	// Make 3 texture objects
	glGenTextures(3, uiTextures);
	
	// Load the Marble
	glBindTexture(GL_TEXTURE_2D, uiTextures[0]);
	LoadTGATexture("marble.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);
	
	// Load Mars
	glBindTexture(GL_TEXTURE_2D, uiTextures[1]);
	LoadTGATexture("marslike.tga", GL_LINEAR_MIPMAP_LINEAR, 
				   GL_LINEAR, GL_CLAMP_TO_EDGE);
	
	// Load Moon
	glBindTexture(GL_TEXTURE_2D, uiTextures[2]);
	LoadTGATexture("moonlike.tga", GL_LINEAR_MIPMAP_LINEAR,
				   GL_LINEAR, GL_CLAMP_TO_EDGE);
}

void CheckErrors(GLuint progName = 0)
{
	GLenum error = glGetError();
		
	if (error != GL_NO_ERROR)
	{
		cout << "A GL Error has occured\n";
	}
	
	GLenum fboStatus = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);

	if(fboStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		switch (fboStatus)
		{
		case GL_FRAMEBUFFER_UNDEFINED:
			// Oops, no window exists?
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			// Check the status of each attachment
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			// Attach at least one buffer to the FBO
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			// Check that all attachments enabled via
			// glDrawBuffers exist in FBO
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			// Check that the buffer specified via
			// glReadBuffer exists in FBO
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			// Reconsider formats used for attached buffers
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			// Make sure the number of samples for each 
			// attachment is the same 
			break;
		//case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
			// Make sure the number of layers for each 
			// attachment is the same 
			//break;
		}
		cout << "The framebuffer is not complete\n";
	}

	if (progName != 0)
	{
		glValidateProgram(progName);
		int iIsProgValid = 0;
		glGetProgramiv(progName, GL_VALIDATE_STATUS, &iIsProgValid);
		if(iIsProgValid == 0)
		{
			cout << "The current program is not valid\n";
		}
	}
}

        
// Called to draw scene
void RenderScene(void)
{
    if(!g_ContinueRendering)
        return;

	static CStopWatch	rotTimer;
	float yRot = rotTimer.GetElapsedSeconds() * 60.0f;
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	modelViewMatrix.PushMatrix();	
	M3DMatrix44f mCamera;
	cameraFrame.GetCameraMatrix(mCamera);
	modelViewMatrix.MultMatrix(mCamera);
	
	// Draw the world upside down
	modelViewMatrix.PushMatrix();
	modelViewMatrix.Scale(1.0f, -1.0f, 1.0f); // Flips the Y Axis
	modelViewMatrix.Translate(0.0f, 0.8f, 0.0f); // Scootch the world down a bit...
	glFrontFace(GL_CW);
	DrawSongAndDance(yRot);
	glFrontFace(GL_CCW);
	modelViewMatrix.PopMatrix();
	
	// Draw the solid ground
	glEnable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, uiTextures[0]);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	static GLfloat vFloorColor[] = { 1.0f, 1.0f, 1.0f, 0.75f};
	shaderManager.UseStockShader(GLT_SHADER_TEXTURE_MODULATE,
								 transformPipeline.GetModelViewProjectionMatrix(),
								 vFloorColor,
								 0);
	
	floorBatch.Draw();
	glDisable(GL_BLEND);
	
	DrawSongAndDance(yRot);
	
	modelViewMatrix.PopMatrix();
       
    // Do the buffer Swap
    SwapBuffers(g_hDC);

    CheckErrors();
}



// Respond to arrow keys by moving the camera frame of reference
void SpecialKeys(int key)
    {
	float linear = 0.1f;
	float angular = float(m3dDegToRad(5.0f));
	
	if(key == VK_UP)
		cameraFrame.MoveForward(linear);
	
	if(key == VK_DOWN)
		cameraFrame.MoveForward(-linear);
	
	if(key == VK_LEFT)
		cameraFrame.RotateWorld(angular, 0.0f, 1.0f, 0.0f);
	
	if(key == VK_RIGHT)
		cameraFrame.RotateWorld(-angular, 0.0f, 1.0f, 0.0f);		
    
	if(key == VK_ESCAPE ||key == 'q' || key == 'Q')
    {
        g_ContinueRendering = false;
        PostQuitMessage(0);
    }
}

void ChangeSize(int nWidth, int nHeight)
{
	glViewport(0, 0, nWidth, nHeight);
	transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
	
	viewFrustum.SetPerspective(35.0f, float(nWidth)/float(nHeight), 1.0f, 100.0f);
	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	modelViewMatrix.LoadIdentity();
}

///////////////////////////////////////////////////////////////////////////////
// Callback functions to handle all window functions this app cares about.
// Once complete, pass message on to next app in the hook chain.
LRESULT CALLBACK WndProc(	HWND	hWnd,		// Handle For This Window
                            UINT	uMsg,		// Message For This Window
                            WPARAM	wParam,		// Additional Message Information
                            LPARAM	lParam)		// Additional Message Information
{
    // Handle relevant messages individually
    switch(uMsg)
    {
    case WM_ACTIVATE:
    case WM_SETFOCUS:
        RenderScene();
        return 0;
    case WM_SIZE:
        ChangeSize(LOWORD(lParam),HIWORD(lParam));
        RenderScene();
        break;
    case WM_CLOSE:
        g_ContinueRendering = false;
        PostQuitMessage(0);
        return 0;
    case WM_KEYDOWN:
        SpecialKeys((unsigned int)wParam);
        return 0;
    default:
        // Nothing to do now
        break;
    }

    // Pass All Unhandled Messages To DefWindowProc
    return DefWindowProc(hWnd,uMsg,wParam,lParam);
}



///////////////////////////////////////////////////////////////////////////////
// Setup the actual window and related state.
// Create the window, find a pixel format, create the OpenGL context
bool SetupWindow(int nWidth, int nHeight, bool bUseFS)
{
    bool bRetVal = true;

    int nWindowX = 0;
    int nWindowY = 0;
    int nPixelFormat  = -1;
    PIXELFORMATDESCRIPTOR pfd;

    DWORD dwExtStyle;
    DWORD dwWindStyle;

    HINSTANCE g_hInstance = GetModuleHandle(NULL);

    // setup window class
    g_windClass.lpszClassName = g_szClassName;                // Set the name of the Class
    g_windClass.lpfnWndProc   = (WNDPROC)WndProc;
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

    // Now that extensions are setup, delete window and start over picking a real format.
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(g_hRC);
    ReleaseDC(g_hWnd, g_hDC);
    DestroyWindow(g_hWnd);

    if(bUseFS)
    {
        // Prepare for a mode set to the requested resolution
        DEVMODE dm;					
		memset(&dm,0,sizeof(dm));	
		dm.dmSize=sizeof(dm);		
		dm.dmPelsWidth	= nWidth;	
		dm.dmPelsHeight	= nHeight;	
		dm.dmBitsPerPel	= 32;		
		dm.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

        long error = ChangeDisplaySettings(&dm, CDS_FULLSCREEN);
        
        if (error != DISP_CHANGE_SUCCESSFUL)
		{
            // Oops, something went wrong, let the user know.
            if (MessageBox(NULL, "Could not set fullscreen mode.\n"
                "Your video card may not support the requested mode.\n"
                "Use windowed mode instead?", 
                g_szAppName, MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
            {
                g_InFullScreen = false;
                dwExtStyle  = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
                dwWindStyle = WS_OVERLAPPEDWINDOW;
			}
			else
			{
				MessageBox(NULL, "Program will exit.", "ERROR", MB_OK|MB_ICONSTOP);
				return false;
			}
        }
        else
        {
            // Mode set passed, setup the styles for fullscreen
            g_InFullScreen = true;
            dwExtStyle  = WS_EX_APPWINDOW;
            dwWindStyle = WS_POPUP;
            ShowCursor(FALSE);
        }
    }

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

    // Uncomment to print a list of all supported pixel formats
    /*    
    // Enumerate supported pixel formats
    GLint pfAttribCount[] = {WGL_NUMBER_PIXEL_FORMATS_ARB};
    GLint pfAttribList[] = { WGL_DRAW_TO_WINDOW_ARB,
                             WGL_ACCELERATION_ARB,
                             WGL_SUPPORT_OPENGL_ARB,
                             WGL_DOUBLE_BUFFER_ARB,
                             WGL_DEPTH_BITS_ARB,
                             WGL_STENCIL_BITS_ARB,
                             WGL_RED_BITS_ARB,
                             WGL_GREEN_BITS_ARB,
                             WGL_BLUE_BITS_ARB,
                             WGL_ALPHA_BITS_ARB
                             };
    int nPixelFormatCount = 0;
    wglGetPixelFormatAttribivARB(g_hDC, 1, 0, 1, pfAttribCount, &nPixelFormatCount);
    for (int i=0; i<nPixelFormatCount; i++)
    {
        GLint results[10];
        printf("Pixel format %d details:\n", nPixelFormatCount);
        wglGetPixelFormatAttribivARB(g_hDC, i, 0, 10, pfAttribList, results);
        printf("    Draw to Window  = %d:\n", results[0]);
        printf("    HW Accelerated  = %d:\n", results[1]);
        printf("    Supports OpenGL = %d:\n", results[2]);
        printf("    Double Buffered = %d:\n", results[3]);
        printf("    Depth Bits   = %d:\n", results[4]);
        printf("    Stencil Bits = %d:\n", results[5]);
        printf("    Red Bits     = %d:\n", results[6]);
        printf("    Green Bits   = %d:\n", results[7]);
        printf("    Blue Bits    = %d:\n", results[8]);
        printf("    Alpha Bits   = %d:\n", results[9]);
    }
    */

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

        // Got a format, now set it as the current one
        SetPixelFormat( g_hDC, nPixelFormat, &pfd );

        GLint attribs[] = {WGL_CONTEXT_MAJOR_VERSION_ARB,  3,
	                       WGL_CONTEXT_MINOR_VERSION_ARB,  3,
                           0 };
        
        g_hRC = wglCreateContextAttribsARB(g_hDC, 0, attribs);
        if (g_hRC == NULL)
        {
            printf("!!! Could not create an OpenGL 3.3 context.\n");
            attribs[3] = 2;
            g_hRC = wglCreateContextAttribsARB(g_hDC, 0, attribs);
            if (g_hRC == NULL)
            {
                printf("!!! Could not create an OpenGL 3.2 context.\n");
                attribs[3] = 1;
                g_hRC = wglCreateContextAttribsARB(g_hDC, 0, attribs);
                if (g_hRC == NULL)
                {
                    printf("!!! Could not create an OpenGL 3.1 context.\n");
                    attribs[3] = 0;
                    g_hRC = wglCreateContextAttribsARB(g_hDC, 0, attribs);
                    if (g_hRC == NULL)
                    {
                        printf("!!! Could not create an OpenGL 3.0 context.\n");
                        printf("!!! OpenGL 3.0 and higher are not supported on this system.\n");
                    }
                }
            }
        }

        wglMakeCurrent( g_hDC, g_hRC );
    }

    if (g_hDC == 0 ||
        g_hDC == 0)
    {
        bRetVal = false;
        printf("!!! An error occured creating an OpenGL window.\n");
    }

    // If everything went as planned, display the window 
    if( bRetVal )
    {
        ShowWindow( g_hWnd, SW_SHOW );
        SetForegroundWindow( g_hWnd );
        SetFocus( g_hWnd );
        g_ContinueRendering = true;
    }
    
    return bRetVal;
}

///////////////////////////////////////////////////////////////////////////////
// Cleanup window, OGL context and related state
// Called on exit and on error
bool KillWindow( )
{ 
    bool bRetVal = true;

    glDeleteTextures(3,uiTextures);
    
    if (g_InFullScreen)
    {
        ChangeDisplaySettings(NULL,0);	
		ShowCursor(TRUE);
    }
    //Cleanup OGL RC
    if(g_hRC) 
    {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(g_hRC);
        g_hRC = NULL;
    }

    // release the DC
    if(g_hDC)
    {
        ReleaseDC(g_hWnd, g_hDC);
        g_hDC = NULL;
    }

    // Destroy the window
    if(g_hWnd)
    {
        DestroyWindow(g_hWnd);
        g_hWnd = NULL;;
    }

    // Delete the window class
    UnregisterClass(g_szClassName, g_hInstance);
    g_hInstance = NULL;
    ShowCursor(TRUE);
    return bRetVal;
}

///////////////////////////////////////////////////////////////////////////////
// Main rendering loop
// Check for window messages and handle events, also draw scene
void mainLoop()
{
    MSG		msg;

    // Check for waiting mssgs
    if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
    {
	    if (msg.message==WM_QUIT)
	    {
            g_ContinueRendering = false;
	    }
	    else
	    {
            // Deal with mssgs
		    TranslateMessage(&msg);
		    DispatchMessage(&msg);
	    }
    }
    else
    {
        RenderScene();       
    }
}

///////////////////////////////////////////////////////////////////////////////
// Main program function, called on startup
// First setup the window and OGL state, then enter rendering loop
int main(int argc, char* argv[])
{
    gltSetWorkingDirectory(argv[0]);
        
    bool bUseFS = false;
    if (MessageBox(NULL, "Would you like to run in Fullscreen Mode?", g_szAppName, MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
    {
        bUseFS = true;
    }

    if(SetupWindow(g_nWinWidth, g_nWinHeight, bUseFS))
    {
        SetupRC();
        ChangeSize(g_nWinWidth, g_nWinHeight);
        while (g_ContinueRendering)
        {   
            mainLoop();
            Sleep(0);
        }
    }
    KillWindow();
	return 0;
}

