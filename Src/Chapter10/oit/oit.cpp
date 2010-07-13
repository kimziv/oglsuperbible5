#include <stdio.h>
#include <iostream>

#include <GLTools.h>
#include <GLShaderManager.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>

#include <GL\glu.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <gl/glut.h>
#endif

#pragma warning( disable : 4305 )

static GLfloat vLtBlue[]    = { 0.00f, 0.00f, 1.00f, 0.90f };
static GLfloat vLtPink[]    = { 0.40f, 0.00f, 0.20f, 0.50f };
static GLfloat vLtYellow[]  = { 0.98f, 0.96f, 0.14f, 0.30f };
static GLfloat vLtMagenta[] = { 0.83f, 0.04f, 0.83f, 0.70f };
static GLfloat vLtGreen[]   = { 0.05f, 0.98f, 0.14f, 0.30f };

static GLfloat vGrey[]   = { 0.5f, 0.5f, 0.5f, 1.0f };

#define USER_OIT   1 
#define USER_BLEND 2

GLsizei	 screenWidth;			// Desired window or desktop width
GLsizei  screenHeight;			// Desired window or desktop height

GLboolean bFullScreen;			// Request to run full screen
GLboolean bAnimated;			// Request for continual updates


GLShaderManager		shaderManager;			// Shader Manager
GLMatrixStack		modelViewMatrix;		// Modelview Matrix
GLMatrixStack		projectionMatrix;		// Projection Matrix
GLFrustum			viewFrustum;			// View Frustum
GLGeometryTransform	transformPipeline;		// Geometry Transform Pipeline
GLFrame				cameraFrame;			// Camera frame

GLTriangleBatch		bckgrndCylBatch;
GLTriangleBatch		diskBatch;
GLBatch				glass1Batch;
GLBatch				glass2Batch;
GLBatch				glass3Batch;
GLBatch				glass4Batch;
GLBatch             screenQuad;
M3DMatrix44f        orthoMatrix;  
GLfloat             worldAngle;

GLint               blendMode;
GLint               mode;

GLuint              msFBO;
GLuint				textures[2];
GLuint				msTexture[1];
GLuint              depthTextureName; 
GLuint              msResolve;
GLuint              oitResolve;
GLuint              flatBlendProg;

void MoveCamera(void);
void DrawWorld();
bool LoadBMPTexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode);
void GenerateOrtho2DMat(GLuint imageWidth, GLuint imageHeight);
void SetupResolveProg();
void SetupOITResolveProg();


///////////////////////////////////////////////////////////////////////////////////////////////////////
// Load in a BMP file as a texture. Allows specification of the filters and the wrap mode
bool LoadBMPTexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)	
{
    BYTE *pBits;
    GLint iWidth, iHeight;

    pBits = gltReadBMPBits(szFileName, &iWidth, &iHeight);
    if(pBits == NULL)
        return false;

    // Set Wrap modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

    // Do I need to generate mipmaps?
    if(minFilter == GL_LINEAR_MIPMAP_LINEAR || minFilter == GL_LINEAR_MIPMAP_NEAREST || minFilter == GL_NEAREST_MIPMAP_LINEAR || minFilter == GL_NEAREST_MIPMAP_NEAREST)
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, iWidth, iHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, pBits);
    return true;
}


///////////////////////////////////////////////////////////////////////////////
// OpenGL related startup code is safe to put here. Load textures, etc.
void SetupRC(void)
{
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }

    // Initialze Shader Manager
    shaderManager.InitializeStockShaders();
    glEnable(GL_DEPTH_TEST);

    gltMakeCylinder(bckgrndCylBatch, 4.0, 4.0, 5.2, 1024, 1);

    gltMakeDisk(diskBatch, 0.0, 1.5, 40, 10);

    glass1Batch.Begin(GL_TRIANGLE_FAN, 4, 1);
        glass1Batch.Vertex3f(-1.0f, -1.0f, 0.0f);
        glass1Batch.Vertex3f( 1.0f, -1.0f, 0.0f);
        glass1Batch.Vertex3f( 1.0f,  1.0f, 0.0f);
        glass1Batch.Vertex3f(-1.0f,  1.0f, 0.0f);
    glass1Batch.End();

    glass2Batch.Begin(GL_TRIANGLE_FAN, 4, 1);
        glass2Batch.Vertex3f( 0.0f,  1.0f, 0.0f);
        glass2Batch.Vertex3f( 1.0f,  0.0f, 0.0f);
        glass2Batch.Vertex3f( 0.0f, -1.0f, 0.0f);
        glass2Batch.Vertex3f(-1.0f,  0.0f, 0.0f);
    glass2Batch.End();
        
    glass3Batch.Begin(GL_TRIANGLE_FAN, 3, 1);
        glass3Batch.Vertex3f( 0.0f,  1.0f, 0.0f);
        glass3Batch.Vertex3f( 1.0f, -1.0f, 0.0f);
        glass3Batch.Vertex3f(-1.0f,  -1.0f, 0.0f);
    glass3Batch.End();

    glass4Batch.Begin(GL_TRIANGLE_FAN, 4, 1);
        glass4Batch.Vertex3f(-1.0f,  1.0f, 0.0f);
        glass4Batch.Vertex3f( 1.0f,  0.5f, 0.0f);
        glass4Batch.Vertex3f( 1.0f, -1.0f, 0.0f);
        glass4Batch.Vertex3f(-1.0f, -0.5f, 0.0f);
    glass4Batch.End();

    glGenTextures(2, textures);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    LoadBMPTexture("marble.bmp", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    LoadBMPTexture("start_line.bmp", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);

    // Create and bind an FBO
    glGenFramebuffers(1,&msFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msFBO);

    // Create depth texture
    glGenTextures(1, &depthTextureName);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depthTextureName);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, GL_DEPTH_COMPONENT24, screenWidth, screenHeight, GL_FALSE);
             
    // Setup HDR render texture
    glGenTextures(1, msTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msTexture[0]);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, GL_RGBA8, screenWidth, screenHeight, GL_FALSE);
    
    // Create and bind an FBO
    glGenFramebuffers(1, &msFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msFBO);

    // Attach texture to first color attachment and the depth RBO
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msTexture[0], 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depthTextureName, 0);

    // Reset framebuffer binding
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    // Load oit resolve shader
    oitResolve =  gltLoadShaderPairWithAttributes("basic.vs", "oitResolve.fs", 3, 
                            GLT_ATTRIBUTE_VERTEX, "vVertex", 
                            GLT_ATTRIBUTE_NORMAL, "vNormal", 
                            GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");
    glBindFragDataLocation(oitResolve, 0, "oColor");
    glLinkProgram(oitResolve);

	// Load multisample resolve shader
    msResolve =  gltLoadShaderPairWithAttributes("basic.vs", "msResolve.fs", 3, 
                            GLT_ATTRIBUTE_VERTEX, "vVertex", 
                            GLT_ATTRIBUTE_NORMAL, "vNormal", 
                            GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");

    glBindFragDataLocation(msResolve, 0, "oColor");
    glLinkProgram(msResolve);

    // Make sure all went well
    gltCheckErrors(oitResolve);
    gltCheckErrors(msResolve);
    
    int numMasks = 0;
    glGetIntegerv(GL_MAX_SAMPLE_MASK_WORDS, &numMasks);
}


///////////////////////////////////////////////////////////////////////////////
// Do your cleanup here. Free textures, display lists, buffer objects, etc.
void ShutdownRC(void)
{
    // Make sure default FBO is bound
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    // Cleanup textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glDeleteTextures(1, msTexture);
    glDeleteTextures(1, &depthTextureName);
    glDeleteTextures(1, textures);

    // Cleanup FBOs
    glDeleteFramebuffers(1, &msFBO);

}


///////////////////////////////////////////////////////////////////////////////
// This is called at least once and before any rendering occurs. If the screen
// is a resizeable window, then this will also get called whenever the window
// is resized.
void ChangeSize(int nWidth, int nHeight)
{
    glViewport(0, 0, nWidth, nHeight);
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
 
    viewFrustum.SetPerspective(35.0f, float(nWidth)/float(nHeight), 1.0f, 100.0f);
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    modelViewMatrix.LoadIdentity();

    GenerateOrtho2DMat(nWidth, nHeight);

    // update screen sizes
    screenWidth = nWidth;
    screenHeight = nHeight;

    // Resize textures
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depthTextureName);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, GL_DEPTH_COMPONENT24, screenWidth, screenHeight, GL_FALSE);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msTexture[0]);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, GL_RGBA8, screenWidth, screenHeight, GL_FALSE);
}


///////////////////////////////////////////////////////////////////////////////
// Update the camera based on user input, toggle display modes
// 
void MoveCamera(void)
{ 
    static CStopWatch cameraTimer;
    float fTime = cameraTimer.GetElapsedSeconds();
    cameraTimer.Reset(); 

    float linear = fTime * 3.0f;
    float angular = fTime * float(m3dDegToRad(60.0f));

    if(GetAsyncKeyState(VK_LEFT))
    {
        worldAngle += angular*50;
        if(worldAngle > 360)
            worldAngle -= 360;
    }

    if(GetAsyncKeyState(VK_RIGHT))
    {
        worldAngle -= angular*50;
        if(worldAngle < 360)
            worldAngle += 360;
    }
    if(GetAsyncKeyState('o') || GetAsyncKeyState('O'))
        mode = USER_OIT;
    if(GetAsyncKeyState('b') || GetAsyncKeyState('B'))
    	mode = USER_BLEND;

    if(GetAsyncKeyState('1'))
    	blendMode = 1;
    if(GetAsyncKeyState('2'))
    	blendMode = 2;
    if(GetAsyncKeyState('3'))
    	blendMode = 3;
    if(GetAsyncKeyState('4'))
    	blendMode = 4;
    if(GetAsyncKeyState('5'))
    	blendMode = 5;
    if(GetAsyncKeyState('6'))
    	blendMode = 6;
    if(GetAsyncKeyState('7'))
    	blendMode = 7;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////
// Create a matrix that maps geometry to the screen. 1 unit in the x directionequals one pixel 
// of width, same with the y direction.
//
void GenerateOrtho2DMat(GLuint imageWidth, GLuint imageHeight)
{
    float right = (float)imageWidth;
    float quadWidth = right;
    float left  = 0.0f;
    float top = (float)imageHeight;
    float quadHeight = top;
    float bottom = 0.0f;

    // set ortho matrix
    orthoMatrix[0] = (float)(2 / (right));
    orthoMatrix[1] = 0.0;
    orthoMatrix[2] = 0.0;
    orthoMatrix[3] = 0.0;

    orthoMatrix[4] = 0.0;
    orthoMatrix[5] = (float)(2 / (top));
    orthoMatrix[6] = 0.0;
    orthoMatrix[7] = 0.0;

    orthoMatrix[8] = 0.0;
    orthoMatrix[9] = 0.0;
    orthoMatrix[10] = (float)(-2 / (1.0 - 0.0));
    orthoMatrix[11] = 0.0;

    orthoMatrix[12] = -1.0f;
    orthoMatrix[13] = -1.0f;
    orthoMatrix[14] = -1.0f;
    orthoMatrix[15] =  1.0;

    // set screen quad vertex array
    screenQuad.Reset();
    screenQuad.Begin(GL_TRIANGLE_STRIP, 4, 1);
        screenQuad.Color4f(0.0f, 1.0f, 0.0f, 1.0f);
        screenQuad.MultiTexCoord2f(0, 0.0f, 0.0f); 
        screenQuad.Vertex3f(0.0f, 0.0f, 0.0f);

        screenQuad.Color4f(0.0f, 1.0f, 0.0f, 1.0f);
        screenQuad.MultiTexCoord2f(0, 1.0f, 0.0f);
        screenQuad.Vertex3f(right, 0.0f, 0.0f);

        screenQuad.Color4f(0.0f, 1.0f, 0.0f, 1.0f);
        screenQuad.MultiTexCoord2f(0, 0.0f, 1.0f);
        screenQuad.Vertex3f(0.0f, top, 0.0f);

        screenQuad.Color4f(0.0f, 1.0f, 0.0f, 1.0f);
        screenQuad.MultiTexCoord2f(0, 1.0f, 1.0f);
        screenQuad.Vertex3f(right, top, 0.0f);
    screenQuad.End();
}


void SetupResolveProg()
{
    glUseProgram(msResolve);

    // Set projection matrix
    glUniformMatrix4fv(glGetUniformLocation(msResolve, "pMatrix"), 
        1, GL_FALSE, transformPipeline.GetProjectionMatrix());

    // Set MVP matrix
    glUniformMatrix4fv(glGetUniformLocation(msResolve, "mvMatrix"), 
        1, GL_FALSE, transformPipeline.GetModelViewMatrix());

    // Now setup the right textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msTexture[0]);
    glUniform1i(glGetUniformLocation(msResolve, "origImage"), 0);

    glUniform1i(glGetUniformLocation(msResolve, "sampleCount"), 8);
    
    glActiveTexture(GL_TEXTURE0);
	    
	gltCheckErrors(msResolve);
} 

void SetupOITResolveProg()
{
    glUseProgram(oitResolve);

    // Set projection matrix
    glUniformMatrix4fv(glGetUniformLocation(oitResolve, "pMatrix"), 
        1, GL_FALSE, transformPipeline.GetProjectionMatrix());

    // Set MVP matrix
    glUniformMatrix4fv(glGetUniformLocation(oitResolve, "mvMatrix"), 
        1, GL_FALSE, transformPipeline.GetModelViewMatrix());

    // Now setup the right textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msTexture[0]);
    glUniform1i(glGetUniformLocation(oitResolve, "origImage"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depthTextureName);
    glUniform1i(glGetUniformLocation(oitResolve, "origDepth"), 1);

    glUniform1f(glGetUniformLocation(oitResolve, "sampleCount"), 8);
    
    glActiveTexture(GL_TEXTURE0);
    gltCheckErrors(oitResolve);
} 

///////////////////////////////////////////////////////////////////////////////
// Draw the scene 
// 
void DrawWorld()
{
    modelViewMatrix.Translate(0.0f, 0.8f, 0.0f);
    modelViewMatrix.PushMatrix();
        modelViewMatrix.Translate(-0.3f, 0.f, 0.0f);
        modelViewMatrix.Scale(0.40, 0.8, 0.40);
        modelViewMatrix.Rotate(50.0, 0.0, 10.0, 0.0);
        glSampleMaski(0, 0x02);
        shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vLtYellow);
        glass1Batch.Draw();
    modelViewMatrix.PopMatrix();

    modelViewMatrix.PushMatrix();
        modelViewMatrix.Translate(0.4f, 0.0f, 0.0f);
        modelViewMatrix.Scale(0.5, 0.8, 1.0);
        modelViewMatrix.Rotate(-20.0, 0.0, 1.0, 0.0);
        glSampleMaski(0, 0x04);
        shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vLtGreen);
        glass2Batch.Draw();
    modelViewMatrix.PopMatrix();

    modelViewMatrix.PushMatrix();
        modelViewMatrix.Translate(1.0f, 0.0f, -0.6f);
        modelViewMatrix.Scale(0.3, 0.9, 1.0);
        modelViewMatrix.Rotate(-40.0, 0.0, 1.0, 0.0);
        glSampleMaski(0, 0x08);
        shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vLtMagenta);
        glass3Batch.Draw();
    modelViewMatrix.PopMatrix();

    modelViewMatrix.PushMatrix();
        modelViewMatrix.Translate(-0.8f, 0.0f, -0.60f);
        modelViewMatrix.Scale(0.6, 0.9, 0.40);
        modelViewMatrix.Rotate(60.0, 0.0, 1.0, 0.0);
        glSampleMaski(0, 0x10);
        shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vLtBlue);
        glass4Batch.Draw();
    modelViewMatrix.PopMatrix();

    modelViewMatrix.PushMatrix();
        modelViewMatrix.Translate(0.1f, 0.0f, 0.50f);
        modelViewMatrix.Scale(0.4, 0.9, 0.4);
        modelViewMatrix.Rotate(205.0, 0.0, 1.0, 0.0);
        glSampleMaski(0, 0x20);
        shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vLtPink);
        glass4Batch.Draw();
    modelViewMatrix.PopMatrix();
}

///////////////////////////////////////////////////////////////////////////////
// Render a frame. The owning framework is responsible for buffer swaps,
// flushes, etc.
void RenderScene(void)
{
    MoveCamera();
    
    // Bind the FBO with multisample buffers
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msFBO);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// User selected order independant transparency
    if (mode == USER_OIT)
    {
        // Use OIT, setup sample masks
        glSampleMaski(0, 0x01);
        glEnable(GL_SAMPLE_MASK);

		// Prevent depth test from culling covered surfaces
        glDepthFunc(GL_ALWAYS);
    }
    
    modelViewMatrix.PushMatrix();	
      M3DMatrix44f mCamera;
      cameraFrame.GetCameraMatrix(mCamera);
      modelViewMatrix.MultMatrix(mCamera);

      modelViewMatrix.PushMatrix();	
        modelViewMatrix.Translate(0.0f, -0.4f, -4.0f);
        modelViewMatrix.Rotate(worldAngle, 0.0, 1.0, 0.0);

		// Draw the background and disk to the first sample
        modelViewMatrix.PushMatrix();
          modelViewMatrix.Translate(0.0f, 3.0f, 0.0f);
          modelViewMatrix.Rotate(90.0, 1.0, 0.0, 0.0);
          modelViewMatrix.Rotate(90.0, 0.0, 0.0, 1.0);
          glBindTexture(GL_TEXTURE_2D, textures[1]); 
          shaderManager.UseStockShader(GLT_SHADER_TEXTURE_REPLACE, transformPipeline.GetModelViewProjectionMatrix(), 0);
          bckgrndCylBatch.Draw();
        modelViewMatrix.PopMatrix();
        
        modelViewMatrix.Translate(0.0f, -0.3f, 0.0f);
        modelViewMatrix.PushMatrix();
            modelViewMatrix.Rotate(90.0, 1.0, 0.0, 0.0);
            shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGrey);
            diskBatch.Draw();
        modelViewMatrix.PopMatrix();
		modelViewMatrix.Translate(0.0f, 0.1f, 0.0f);

		// User selected blending
        if (mode == USER_BLEND)
        {
            // Setup blend state
			glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            switch (blendMode)
            {
            case 1:
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                break;
            case 2:
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA);
                break;
            case 3:
                glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                break;
            case 4:
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                break;
            case 5:
                glBlendFunc(GL_SRC_ALPHA, GL_DST_COLOR);
                break;
            case 6:
                glBlendFuncSeparate(GL_SRC_ALPHA, GL_DST_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                break;
            case 7:
                glBlendFuncSeparate(GL_SRC_COLOR, GL_DST_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                break;
            default:
                glDisable(GL_BLEND);
            }
        }
   
		// Now draw the glass pieces
        DrawWorld();
    
      modelViewMatrix.PopMatrix();
    modelViewMatrix.PopMatrix();
    
    // Clean up all state 
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_BLEND);
    glDisable(GL_SAMPLE_MASK);
    glSampleMaski(0, 0xffffffff);

    // Resolve multisample buffer
    projectionMatrix.PushMatrix();
      projectionMatrix.LoadMatrix(orthoMatrix);
      modelViewMatrix.PushMatrix();
        modelViewMatrix.LoadIdentity();
		// Setup and Clear the default framebuffer
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glViewport(0, 0, screenWidth, screenHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        if (mode == USER_OIT)
            SetupOITResolveProg();
        else if (mode == USER_BLEND)
            SetupResolveProg();

		// Draw a full-size quad to resolve the multisample surfaces
        screenQuad.Draw();
      modelViewMatrix.PopMatrix();
    projectionMatrix.PopMatrix();
    
	// Reset texture state
    glEnable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
               
    // Do the buffer Swap
    glutSwapBuffers();
        
    // Do it again
    glutPostRedisplay();
}

int main(int argc, char* argv[])
{
    
    screenWidth = 800;
    screenHeight = 600; 
    msFBO = 0;
    depthTextureName = 0;
    worldAngle = 0;
    mode = 1;
    blendMode = 1;

	gltSetWorkingDirectory(argv[0]);
		
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(screenWidth,screenHeight);
  
    glutCreateWindow("HDR Imaging");
 
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);

    SetupRC();
    glutMainLoop();    
    ShutdownRC();
    return 0;
}