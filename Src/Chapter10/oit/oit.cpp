#include "oit.h"
#include <GL\glu.h>
#include <stdio.h>
#include <iostream>

#pragma warning( disable : 4305 )

static GLfloat vRed[]   = { 1.0f, 0.0f, 0.0f, 1.0f };
static GLfloat vGreen[]  = { 0.0f, 1.0f, 0.0f, 1.0f };
static GLfloat vBlue[]   = { 0.0f, 0.0f, 1.0f, 1.0f };

static GLfloat vLtBlue[]    = { 0.00f, 0.00f, 1.00f, 0.90f };
static GLfloat vLtPink[]    = { 0.40f, 0.00f, 0.20f, 0.50f };
static GLfloat vLtYellow[]  = { 0.98f, 0.96f, 0.14f, 0.30f };
static GLfloat vLtMagenta[] = { 0.83f, 0.04f, 0.83f, 0.70f };
static GLfloat vLtGreen[]   = { 0.05f, 0.98f, 0.14f, 0.30f };

static GLfloat vWhite[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
static GLfloat vBlack[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
static GLfloat vGrey[]   = { 0.5f, 0.5f, 0.5f, 1.0f };


////////////////////////////////////////////////////////////////////////////
// Do not put any OpenGL code here. General guidence on constructors in 
// general is to not put anything that can fail here either (opening files,
// allocating memory, etc.)
OrderIndependentTransparancy::OrderIndependentTransparancy(void) : screenWidth(800), screenHeight(600), bFullScreen(false), 
                bAnimated(true), msFBO(0), depthTextureName(0), worldAngle(0), mode(1), blendMode(1)
{

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
GLint mode = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Load in a BMP file as a texture. Allows specification of the filters and the wrap mode
bool OrderIndependentTransparancy::LoadBMPTexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)	
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
void OrderIndependentTransparancy::Initialize(void)
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

    // Load resolve shader
    msResolve =  gltLoadShaderPairWithAttributes("basic.vs", "msResolve.fs", 3, 
                            GLT_ATTRIBUTE_VERTEX, "vVertex", 
                            GLT_ATTRIBUTE_NORMAL, "vNormal", 
                            GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");
    glBindFragDataLocation(msResolve, 0, "oColor");
    glLinkProgram(msResolve);

    // Make sure all went well
    CheckErrors(msResolve);
    int numMasks = 0;
    glGetIntegerv(GL_MAX_SAMPLE_MASK_WORDS, &numMasks);
}


///////////////////////////////////////////////////////////////////////////////
// Do your cleanup here. Free textures, display lists, buffer objects, etc.
void OrderIndependentTransparancy::Shutdown(void)
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
void OrderIndependentTransparancy::Resize(GLsizei nWidth, GLsizei nHeight)
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
void OrderIndependentTransparancy::MoveCamera(void)
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
        mode = 1;
    if(GetAsyncKeyState('b') || GetAsyncKeyState('B'))
    	mode = 2;

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
void OrderIndependentTransparancy::GenerateOrtho2DMat(GLuint imageWidth, GLuint imageHeight)
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


void OrderIndependentTransparancy::SetupResolveProg()
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
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depthTextureName);
    int depthLoc =glGetUniformLocation(msResolve, "origDepth");
    glUniform1i(depthLoc, 1);
    
    CheckErrors(msResolve);

    glActiveTexture(GL_TEXTURE0);
} 

///////////////////////////////////////////////////////////////////////////////
// Draw the scene 
// 
void OrderIndependentTransparancy::DrawWorld()
{
    modelViewMatrix.Translate(0.0f, 0.8f, 0.0f);
    modelViewMatrix.PushMatrix();
        modelViewMatrix.Translate(-0.3f, 0.f, 0.0f);
        modelViewMatrix.Scale(0.40, 0.8, 0.40);
        modelViewMatrix.Rotate(50.0, 0.0, 10.0, 0.0);
        glSampleMaski(0, 0x02020202);
        shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vLtYellow);
        glass1Batch.Draw();
    modelViewMatrix.PopMatrix();

    modelViewMatrix.PushMatrix();
        modelViewMatrix.Translate(0.4f, 0.0f, 0.0f);
        modelViewMatrix.Scale(0.5, 0.8, 1.0);
        modelViewMatrix.Rotate(-20.0, 0.0, 1.0, 0.0);
        glSampleMaski(0, 0x04040404);
        shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vLtGreen);
        glass2Batch.Draw();
    modelViewMatrix.PopMatrix();

    modelViewMatrix.PushMatrix();
        modelViewMatrix.Translate(1.0f, 0.0f, -1.0f);
        modelViewMatrix.Scale(0.3, 0.9, 1.0);
        modelViewMatrix.Rotate(-40.0, 0.0, 1.0, 0.0);
        glSampleMaski(0, 0x08080808);
        shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vLtMagenta);
        glass3Batch.Draw();
    modelViewMatrix.PopMatrix();

    modelViewMatrix.PushMatrix();
        modelViewMatrix.Translate(-0.8f, 0.0f, -1.0f);
        modelViewMatrix.Scale(0.6, 0.9, 0.40);
        modelViewMatrix.Rotate(60.0, 0.0, 1.0, 0.0);
        glSampleMaski(0, 0x10101010);
        shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vLtBlue);
        glass4Batch.Draw();
    modelViewMatrix.PopMatrix();

    modelViewMatrix.PushMatrix();
        modelViewMatrix.Translate(0.1f, 0.0f, 0.50f);
        modelViewMatrix.Scale(0.4, 0.9, 0.4);
        modelViewMatrix.Rotate(205.0, 0.0, 1.0, 0.0);
        glSampleMaski(0, 0x20202020);
        shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vLtPink);
        glass4Batch.Draw();
    modelViewMatrix.PopMatrix();
}

void OrderIndependentTransparancy::DrawSimpleWorld()
{
    // Set Sample mask
    glSampleMaski(0, 0x01010101);
    //glEnable(GL_SAMPLE_MASK);
    CheckErrors();
    glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);

    // Now draw the mirror surfaces
    modelViewMatrix.PushMatrix();
        modelViewMatrix.Translate(-1.0f, -0.4f, -6.0f);
        shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vLtBlue);
        
        glSampleMaski(0, 0x02020202);
        glass1Batch.Draw();

        modelViewMatrix.Translate(1.0f, 0.0f, 1.0f);
        shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vLtPink);
        glSampleMaski(0, 0x04040404);
        glass1Batch.Draw();

        modelViewMatrix.Translate(1.5f, 0.0f, -1.0f);
        shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vLtBlue);
        glSampleMaski(0, 0x08080808);
        glass1Batch.Draw();
    modelViewMatrix.PopMatrix();

}


///////////////////////////////////////////////////////////////////////////////
// Render a frame. The owning framework is responsible for buffer swaps,
// flushes, etc.
void OrderIndependentTransparancy::Render(void)
{
    MoveCamera();
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);

    // Reset FBO. Draw world again from the real cameras perspective
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (mode == 1)
    {
        // Use OIT, setup sample masks
        glSampleMaski(0, 0x01010101);
        glEnable(GL_SAMPLE_MASK);
    }
    
    modelViewMatrix.PushMatrix();	
      M3DMatrix44f mCamera;
      cameraFrame.GetCameraMatrix(mCamera);
      modelViewMatrix.MultMatrix(mCamera);

      modelViewMatrix.PushMatrix();	
        modelViewMatrix.Translate(0.0f, -0.4f, -4.0f);
        modelViewMatrix.Rotate(worldAngle, 0.0, 1.0, 0.0);

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

        if (mode ==2)
        {
            // Setup blend state
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
        
        // Prevent depth test from culling covered surfaces
        glDisable(GL_DEPTH_TEST);
        glDepthFunc(GL_ALWAYS);

        DrawWorld();
    
      modelViewMatrix.PopMatrix();
    modelViewMatrix.PopMatrix();
    
    // Clean up all state 
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_BLEND);
    glDisable(GL_SAMPLE_MASK);
    glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    glDisable(GL_SAMPLE_COVERAGE);
    glDisable(GL_SAMPLE_SHADING_ARB);
    glSampleMaski(0, 0xffffffff);

    // Resolve multisample buffer
    projectionMatrix.PushMatrix();
      projectionMatrix.LoadMatrix(orthoMatrix);
      modelViewMatrix.PushMatrix();
        modelViewMatrix.LoadIdentity();
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glViewport(0, 0, screenWidth, screenHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        SetupResolveProg();
        screenQuad.Draw();
      modelViewMatrix.PopMatrix();
    projectionMatrix.PopMatrix();
    
    
    glEnable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
}
