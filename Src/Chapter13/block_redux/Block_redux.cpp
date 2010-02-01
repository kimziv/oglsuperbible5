// Block_redux.cpp
// OpenGL SuperBible, Chapter 13
// Demonstrates the basics of setting up a window on a Windows platform
// Program by Richard S. Wright Jr. and Nick Haemel

#include <GLTools.h>	// OpenGL toolkit
#include <GLMatrixStack.h>
#include <GLFrame.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLGeometryTransform.h>
#include <gl\wglew.h>

#include <math.h>

/////////////////////////////////////////////////////////////////////////////////
// An assortment of needed classes
GLShaderManager		shaderManager;
GLMatrixStack		modelViewMatrix;
GLMatrixStack		projectionMatrix;
GLFrame				cameraFrame;
GLFrustum			viewFrustum;
GLBatch				cubeBatch;
GLBatch				floorBatch;
GLBatch				topBlock;
GLBatch				frontBlock;
GLBatch				leftBlock;

GLGeometryTransform	transformPipeline;
M3DMatrix44f		shadowMatrix;

HWND         g_hWnd;
HGLRC        g_hRC;
HDC          g_hDC;
HINSTANCE    g_hInstance;
WNDCLASS     g_windClass; 
RECT         g_windowRect;
bool         g_ContinueRendering;

// Keep track of effects step
int nStep = 0;

// Lighting data
GLfloat lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
GLfloat lightDiffuse[] = { 0.7f, 0.7f, 0.7f, 1.0f };
GLfloat lightSpecular[] = { 0.9f, 0.9f, 0.9f };
GLfloat vLightPos[] = { -8.0f, 20.0f, 100.0f, 1.0f };

GLuint textures[4];


///////////////////////////////////////////////////////////////////////////////
// Make a cube out of a batch of triangles. Texture coordinates and normals
// are also provided.
void MakeCube(GLBatch& cubeBatch)
{
    cubeBatch.Begin(GL_TRIANGLES, 36, 1);

    /////////////////////////////////////////////
    // Top of cube
    cubeBatch.Normal3f(0.0f, 1.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
    cubeBatch.Vertex3f(1.0f, 1.0f, 1.0f);

    cubeBatch.Normal3f(0.0f, 1.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    cubeBatch.Vertex3f(1.0f, 1.0f, -1.0f);

    cubeBatch.Normal3f(0.0f, 1.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    cubeBatch.Vertex3f(-1.0f, 1.0f, -1.0f);

    cubeBatch.Normal3f(0.0f, 1.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
    cubeBatch.Vertex3f(1.0f, 1.0f, 1.0f);

    cubeBatch.Normal3f(0.0f, 1.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    cubeBatch.Vertex3f(-1.0f, 1.0f, -1.0f);

    cubeBatch.Normal3f(0.0f, 1.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
    cubeBatch.Vertex3f(-1.0f, 1.0f, 1.0f);


    ////////////////////////////////////////////
    // Bottom of cube
    cubeBatch.Normal3f(0.0f, -1.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    cubeBatch.Vertex3f(-1.0f, -1.0f, -1.0f);

    cubeBatch.Normal3f(0.0f, -1.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    cubeBatch.Vertex3f(1.0f, -1.0f, -1.0f);

    cubeBatch.Normal3f(0.0f, -1.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
    cubeBatch.Vertex3f(1.0f, -1.0f, 1.0f);

    cubeBatch.Normal3f(0.0f, -1.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
    cubeBatch.Vertex3f(-1.0f, -1.0f, 1.0f);

    cubeBatch.Normal3f(0.0f, -1.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    cubeBatch.Vertex3f(-1.0f, -1.0f, -1.0f);

    cubeBatch.Normal3f(0.0f, -1.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
    cubeBatch.Vertex3f(1.0f, -1.0f, 1.0f);

    ///////////////////////////////////////////
    // Left side of cube
    cubeBatch.Normal3f(-1.0f, 0.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
    cubeBatch.Vertex3f(-1.0f, 1.0f, 1.0f);

    cubeBatch.Normal3f(-1.0f, 0.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    cubeBatch.Vertex3f(-1.0f, 1.0f, -1.0f);

    cubeBatch.Normal3f(-1.0f, 0.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    cubeBatch.Vertex3f(-1.0f, -1.0f, -1.0f);

    cubeBatch.Normal3f(-1.0f, 0.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
    cubeBatch.Vertex3f(-1.0f, 1.0f, 1.0f);

    cubeBatch.Normal3f(-1.0f, 0.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    cubeBatch.Vertex3f(-1.0f, -1.0f, -1.0f);

    cubeBatch.Normal3f(-1.0f, 0.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
    cubeBatch.Vertex3f(-1.0f, -1.0f, 1.0f);

    // Right side of cube
    cubeBatch.Normal3f(1.0f, 0.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    cubeBatch.Vertex3f(1.0f, -1.0f, -1.0f);

    cubeBatch.Normal3f(1.0f, 0.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    cubeBatch.Vertex3f(1.0f, 1.0f, -1.0f);

    cubeBatch.Normal3f(1.0f, 0.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
    cubeBatch.Vertex3f(1.0f, 1.0f, 1.0f);

    cubeBatch.Normal3f(1.0f, 0.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
    cubeBatch.Vertex3f(1.0f, 1.0f, 1.0f);

    cubeBatch.Normal3f(1.0f, 0.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
    cubeBatch.Vertex3f(1.0f, -1.0f, 1.0f);

    cubeBatch.Normal3f(1.0f, 0.0f, 0.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    cubeBatch.Vertex3f(1.0f, -1.0f, -1.0f);

    // Front and Back
    // Front
    cubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    cubeBatch.Vertex3f(1.0f, -1.0f, 1.0f);

    cubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
    cubeBatch.Vertex3f(1.0f, 1.0f, 1.0f);

    cubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
    cubeBatch.Vertex3f(-1.0f, 1.0f, 1.0f);

    cubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
    cubeBatch.Vertex3f(-1.0f, 1.0f, 1.0f);

    cubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    cubeBatch.Vertex3f(-1.0f, -1.0f, 1.0f);

    cubeBatch.Normal3f(0.0f, 0.0f, 1.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    cubeBatch.Vertex3f(1.0f, -1.0f, 1.0f);

    // Back
    cubeBatch.Normal3f(0.0f, 0.0f, -1.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    cubeBatch.Vertex3f(1.0f, -1.0f, -1.0f);

    cubeBatch.Normal3f(0.0f, 0.0f, -1.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    cubeBatch.Vertex3f(-1.0f, -1.0f, -1.0f);

    cubeBatch.Normal3f(0.0f, 0.0f, -1.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
    cubeBatch.Vertex3f(-1.0f, 1.0f, -1.0f);

    cubeBatch.Normal3f(0.0f, 0.0f, -1.0f);
    cubeBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
    cubeBatch.Vertex3f(-1.0f, 1.0f, -1.0f);

    cubeBatch.Normal3f(0.0f, 0.0f, -1.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
    cubeBatch.Vertex3f(1.0f, 1.0f, -1.0f);

    cubeBatch.Normal3f(0.0f, 0.0f, -1.0f);
    cubeBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    cubeBatch.Vertex3f(1.0f, -1.0f, -1.0f);

    cubeBatch.End();
}

/////////////////////////////////////////////////////////////////////////////
// Make the floor, just the verts and texture coordinates, no normals
void MakeFloor(GLBatch& floorBatch)
{
    GLfloat x = 5.0f;
    GLfloat y = -1.0f;

    floorBatch.Begin(GL_TRIANGLE_FAN, 4, 1);
        floorBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
        floorBatch.Vertex3f(-x, y, x);

        floorBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
        floorBatch.Vertex3f(x, y, x);

        floorBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
        floorBatch.Vertex3f(x, y, -x);

        floorBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
        floorBatch.Vertex3f(-x, y, -x);
    floorBatch.End();
}


///////////////////////////////////////////////////////////////////////////////
// This function does any needed initialization on the rendering context. 
// This is the first opportunity to do any OpenGL related tasks.
void SetupRC()
{
    GLbyte *pBytes;
    GLint nWidth, nHeight, nComponents;
    GLenum format;

    shaderManager.InitializeStockShaders();

    // Black background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f );
    glEnable(GL_DEPTH_TEST);
    glLineWidth(2.5f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);

    cameraFrame.MoveForward(-15.0f);
    cameraFrame.MoveUp(6.0f);
    cameraFrame.RotateLocalX(float(m3dDegToRad(20.0f)));
    
    MakeCube(cubeBatch);
    MakeFloor(floorBatch);

    // Make top
    topBlock.Begin(GL_TRIANGLE_FAN, 4, 1);
        topBlock.Normal3f(0.0f, 1.0f, 0.0f);
        topBlock.MultiTexCoord2f(0, 0.0f, 0.0f);
        topBlock.Vertex3f(-1.0f, 1.0f, 1.0f);

        topBlock.Normal3f(0.0f, 1.0f, 0.0f);
        topBlock.MultiTexCoord2f(0, 1.0f, 0.0f);
        topBlock.Vertex3f(1.0f, 1.0f, 1.0f);

        topBlock.Normal3f(0.0f, 1.0f, 0.0f);
        topBlock.MultiTexCoord2f(0, 1.0f, 1.0f);
        topBlock.Vertex3f(1.0f, 1.0f, -1.0f);

        topBlock.Normal3f(0.0f, 1.0f, 0.0f);
        topBlock.MultiTexCoord2f(0, 0.0f, 1.0f);
        topBlock.Vertex3f(-1.0f, 1.0f, -1.0f);
    topBlock.End();

    // Make Front
    frontBlock.Begin(GL_TRIANGLE_FAN, 4, 1);
        frontBlock.Normal3f(0.0f, 0.0f, 1.0f);
        frontBlock.MultiTexCoord2f(0, 0.0f, 0.0f);
        frontBlock.Vertex3f(-1.0f, -1.0f, 1.0f);

        frontBlock.Normal3f(0.0f, 0.0f, 1.0f);
        frontBlock.MultiTexCoord2f(0, 1.0f, 0.0f);
        frontBlock.Vertex3f(1.0f, -1.0f, 1.0f);

        frontBlock.Normal3f(0.0f, 0.0f, 1.0f);
        frontBlock.MultiTexCoord2f(0, 1.0f, 1.0f);
        frontBlock.Vertex3f(1.0f, 1.0f, 1.0f);

        frontBlock.Normal3f(0.0f, 0.0f, 1.0f);
        frontBlock.MultiTexCoord2f(0, 0.0f, 1.0f);
        frontBlock.Vertex3f(-1.0f, 1.0f, 1.0f);
    frontBlock.End();

    // Make left
    leftBlock.Begin(GL_TRIANGLE_FAN, 4, 1);
        leftBlock.Normal3f(-1.0f, 0.0f, 0.0f);
        leftBlock.MultiTexCoord2f(0, 0.0f, 0.0f);
        leftBlock.Vertex3f(-1.0f, -1.0f, -1.0f);

        leftBlock.Normal3f(-1.0f, 0.0f, 0.0f);
        leftBlock.MultiTexCoord2f(0, 1.0f, 0.0f);
        leftBlock.Vertex3f(-1.0f, -1.0f, 1.0f);

        leftBlock.Normal3f(-1.0f, 0.0f, 0.0f);
        leftBlock.MultiTexCoord2f(0, 1.0f, 1.0f);
        leftBlock.Vertex3f(-1.0f, 1.0f, 1.0f);

        leftBlock.Normal3f(-1.0f, 0.0f, 0.0f);
        leftBlock.MultiTexCoord2f(0, 0.0f, 1.0f);
        leftBlock.Vertex3f(-1.0f, 1.0f, -1.0f);
    leftBlock.End();

    // Create shadow projection matrix
    GLfloat floorPlane[] = { 0.0f, 1.0f, 0.0f, 1.0f};
    m3dMakePlanarShadowMatrix(shadowMatrix, floorPlane, vLightPos);

    // Load up four textures  
    glGenTextures(4, textures);
        
    // Wood floor
    pBytes = gltReadTGABits("floor.tga", &nWidth, &nHeight, &nComponents, &format);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D,0,nComponents,nWidth, nHeight, 0,
        format, GL_UNSIGNED_BYTE, pBytes);
    free(pBytes);

    // One of the block faces
    pBytes = gltReadTGABits("Block4.tga", &nWidth, &nHeight, &nComponents, &format);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D,0,nComponents,nWidth, nHeight, 0,
        format, GL_UNSIGNED_BYTE, pBytes);
    free(pBytes);

    // Another block face
    pBytes = gltReadTGABits("block5.tga", &nWidth, &nHeight, &nComponents, &format);
        glBindTexture(GL_TEXTURE_2D, textures[2]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D,0,nComponents,nWidth, nHeight, 0,
        format, GL_UNSIGNED_BYTE, pBytes);
    free(pBytes);

    // Yet another block face
    pBytes = gltReadTGABits("block6.tga", &nWidth, &nHeight, &nComponents, &format);
        glBindTexture(GL_TEXTURE_2D, textures[3]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D,0,nComponents,nWidth, nHeight, 0,
        format, GL_UNSIGNED_BYTE, pBytes);
    free(pBytes);
}

///////////////////////////////////////////////////////////////////////////////
// Render the block
void RenderBlock(void)
{
    GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 1.0f};
    GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    switch(nStep)
        {
        // Wire frame
        case 0:
            glEnable(GL_BLEND);
            glEnable(GL_LINE_SMOOTH);
            shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_CULL_FACE);

            // Draw the cube
            cubeBatch.Draw();

            break;

        // Wire frame, but not the back side... we also want the block to be in the stencil buffer
        case 1:
            shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);

            // Draw (back side) solid block in stencil buffer
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GL_NEVER, 0, 0);
            glStencilOp(GL_INCR, GL_INCR, GL_INCR);
            glFrontFace(GL_CW);
            cubeBatch.Draw();
            glFrontFace(GL_CCW);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            glDisable(GL_STENCIL_TEST);

            glEnable(GL_BLEND);
            glEnable(GL_LINE_SMOOTH);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            // Draw the front side cube
            cubeBatch.Draw();
            break;

        // Solid
        case 2:
            shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
            
            // Draw the cube
            cubeBatch.Draw();
            break;

        // Lit
        case 3:
            shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, modelViewMatrix.GetMatrix(),
                projectionMatrix.GetMatrix(), vLightPos, vRed);

            // Draw the cube
            cubeBatch.Draw();
            break;

        // Textured & Lit
        case 4:
        case 5:
        default:
            glBindTexture(GL_TEXTURE_2D, textures[2]);
            shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF, modelViewMatrix.GetMatrix(),
                projectionMatrix.GetMatrix(), vLightPos, vWhite, 0);

            glBindTexture(GL_TEXTURE_2D, textures[1]);
            topBlock.Draw();
            glBindTexture(GL_TEXTURE_2D, textures[2]);
            frontBlock.Draw();
            glBindTexture(GL_TEXTURE_2D, textures[3]);
            leftBlock.Draw();

            break;
        }
    
    // Put everything back
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_STENCIL_TEST);
}

///////////////////////////////////////////////////////////////////////////////
// Render the floor
void RenderFloor(void)
{
    GLfloat vBrown [] = { 0.55f, 0.292f, 0.09f, 1.0f};
    GLfloat vFloor[] = { 1.0f, 1.0f, 1.0f, 0.6f };

    switch(nStep)
        {
        // Wire frame
        case 0:
            glEnable(GL_BLEND);
            glEnable(GL_LINE_SMOOTH);
            shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBrown);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_CULL_FACE);
            break;

        // Wire frame, but not the back side.. and only where stencil == 0
        case 1:
            glEnable(GL_BLEND);
            glEnable(GL_LINE_SMOOTH);

            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GL_EQUAL, 0, 0xff);

            shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBrown);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            break;

        // Solid
        case 2:
        case 3:	
            shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBrown);
            break;

        // Textured
        case 4:
        case 5:
        default:
            glBindTexture(GL_TEXTURE_2D, textures[0]);
            shaderManager.UseStockShader(GLT_SHADER_TEXTURE_MODULATE, transformPipeline.GetModelViewProjectionMatrix(), vFloor, 0);
            break;
        }
    
    // Draw the floor
    floorBatch.Draw();

    // Put everything back
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_STENCIL_TEST);
}



///////////////////////////////////////////////////////////////////////////////
// Called to draw scene
void RenderScene(void)
{
    if(!g_ContinueRendering)
        return;

    // Clear the window with current clearing color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    modelViewMatrix.PushMatrix();
        M3DMatrix44f mCamera;
        cameraFrame.GetCameraMatrix(mCamera);
        modelViewMatrix.MultMatrix(mCamera);

        // Reflection step... draw cube upside down, the floor
        // blended on top of it
        if(nStep == 5) {
            glDisable(GL_CULL_FACE);
            modelViewMatrix.PushMatrix();
            modelViewMatrix.Scale(1.0f, -1.0f, 1.0f);
            modelViewMatrix.Translate(0.0f, 2.0f, 0.0f);
            modelViewMatrix.Rotate(35.0f, 0.0f, 1.0f, 0.0f);
            RenderBlock();
            modelViewMatrix.PopMatrix();
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            RenderFloor();
            glDisable(GL_BLEND);			
            }

        modelViewMatrix.PushMatrix();

            // Draw normally
            modelViewMatrix.Rotate(35.0f, 0.0f, 1.0f, 0.0f);
            RenderBlock();
        modelViewMatrix.PopMatrix();
        
    // If not the reflection pass, draw floor last
    if(nStep != 5)
        RenderFloor();

    modelViewMatrix.PopMatrix();

    // Flush drawing commands
    SwapBuffers(g_hDC);
}


///////////////////////////////////////////////////////////////////////////////
// Window has changed size, or has just been created. In either case, we need
// to use the window dimensions to set the viewport and the projection matrix.
void ChangeSize(int w, int h)
{
    glViewport(0, 0, w, h);
    viewFrustum.SetPerspective(35.0f, float(w) / float(h), 1.0f, 500.0f);
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
    unsigned int key = 0;
    
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
        key = (unsigned int)wParam;
        if(key == 32)
        {
            nStep++;
            if(nStep > 5)
                nStep = 0;
        }
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
bool SetupWindow(int nWidth, int nHeight)
{
    bool bRetVal = true;

    int nWindowX = 0;
    int nWindowY = 0;
    int nPixelFormat  = -1;
    PIXELFORMATDESCRIPTOR pfd;

    DWORD dwExtStyle;
    DWORD dwWindStyle;

    HINSTANCE g_hInstance = GetModuleHandle(NULL);

    TCHAR szWindowName[50] =  TEXT("Block Redux");
    TCHAR szClassName[50]  =  TEXT("OGL_CLASS");

    // setup window class
    g_windClass.lpszClassName = szClassName;                // Set the name of the Class
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
                            szClassName,    // class name
                            szWindowName,   // window name
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

    // Create the window again
    g_hWnd = CreateWindowEx(dwExtStyle,     // Extended style
                            szClassName,    // class name
                            szWindowName,   // window name
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
                         WGL_RED_BITS_ARB,       8, // 8 bits of red precision in window
                         WGL_GREEN_BITS_ARB,     8, // 8 bits of green precision in window
                         WGL_BLUE_BITS_ARB,      8, // 8 bits of blue precision in window
                         WGL_DEPTH_BITS_ARB,     16, // 16 bits of depth precision for window
                         WGL_PIXEL_TYPE_ARB,      WGL_TYPE_RGBA_ARB, // pf should be RGBA type
                         0}; // NULL termination

    // Ask OpenGL to find the most relevant format matching our attribs
    // Only get one format back.
    wglChoosePixelFormatARB(g_hDC, &pixAttribs[0], NULL, 1, &nPixelFormat, (UINT*)&nPixCount);

    if(nPixelFormat == -1) 
    {
        // Couldn't find a format, perhaps no 3D HW or drivers are installed
        g_hDC = 0;
        g_hDC = 0;
        bRetVal = false;
        printf("!!! An error occurred trying to find a pixel format with the requested attribs.\n");
    }
    else
    {
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

    glDeleteTextures(4,textures);
    
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
    TCHAR szClassName[50]  =  TEXT("OGL_CLASS");
    UnregisterClass(szClassName, g_hInstance);
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
    if(SetupWindow(800, 600))
    {
        SetupRC();
        ChangeSize(800, 600);
        while (g_ContinueRendering)
        {   
            mainLoop();
            Sleep(0);
        }
    }
    KillWindow();
	return 0;
}

