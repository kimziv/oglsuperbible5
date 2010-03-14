#include "fbo_textures.h"
#include <GL\glu.h>
#include <stdio.h>
#include <iostream>

static GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
static GLfloat vBlue[] = { 0.0f, 0.0f, 1.0f, 1.0f };
static GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static GLfloat vBlack[] = { 0.0f, 0.0f, 0.0f, 1.0f };
static GLfloat vGrey[] =  { 0.5f, 0.5f, 0.5f, 1.0f };
static GLfloat vLightPos[] = { -2.0f, 3.0f, -2.0f, 1.0f };
static const GLenum windowBuff[] = { GL_BACK_LEFT };
static const GLenum fboBuffs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
static GLint mirrorTexWidth  = 800;
static GLint mirrorTexHeight = 800;

////////////////////////////////////////////////////////////////////////////
// Do not put any OpenGL code here. General guidence on constructors in 
// general is to not put anything that can fail here either (opening files,
// allocating memory, etc.)
FBOtextures::FBOtextures(void) : screenWidth(800), screenHeight(600), bFullScreen(false), 
				bAnimated(true), fboName(0), depthBufferName(0)
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Load in a BMP file as a texture. Allows specification of the filters and the wrap mode
bool FBOtextures::LoadBMPTexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)	
{
	BYTE *pBits;
	GLint iWidth, iHeight;

	pBits = gltReadBMPBits(szFileName, &iWidth, &iHeight);
	if(pBits == NULL)
		return false;

	// Set Wrap modes
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, iWidth, iHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, pBits);

    // Do I need to generate mipmaps?
	if(minFilter == GL_LINEAR_MIPMAP_LINEAR || minFilter == GL_LINEAR_MIPMAP_NEAREST || minFilter == GL_NEAREST_MIPMAP_LINEAR || minFilter == GL_NEAREST_MIPMAP_NEAREST)
		glGenerateMipmap(GL_TEXTURE_2D);    

	return true;
}


///////////////////////////////////////////////////////////////////////////////
// OpenGL related startup code is safe to put here. Load textures, etc.
void FBOtextures::Initialize(void)
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

	// Black
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	gltMakeTorus(torusBatch, 0.4f, 0.15f, 35, 35);
	gltMakeSphere(sphereBatch, 0.1f, 26, 13);
	gltMakeCylinder(cylinderBatch,0.3f, 0.2f, 1.0, 10,10);

	GLfloat alpha = 0.25f;
	floorBatch.Begin(GL_TRIANGLE_FAN, 4, 1);
		floorBatch.Color4f(0.0f, 1.0f, 0.0f, alpha);
		floorBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
		floorBatch.Normal3f(0.0, 1.0f, 0.0f);
		floorBatch.Vertex3f(-20.0f, -0.41f, 20.0f);

		floorBatch.Color4f(0.0f, 1.0f, 0.0f, alpha);
		floorBatch.MultiTexCoord2f(0, 10.0f, 0.0f);
		floorBatch.Normal3f(0.0, 1.0f, 0.0f);
		floorBatch.Vertex3f(20.0f, -0.41f, 20.0f);

		floorBatch.Color4f(0.0f, 1.0f, 0.0f, alpha);
		floorBatch.MultiTexCoord2f(0, 10.0f, 10.0f);
		floorBatch.Normal3f(0.0, 1.0f, 0.0f);
		floorBatch.Vertex3f(20.0f, -0.41f, -20.0f);

		floorBatch.Color4f(0.0f, 1.0f, 0.0f, alpha);
		floorBatch.MultiTexCoord2f(0, 0.0f, 10.0f);
		floorBatch.Normal3f(0.0, 1.0f, 0.0f);
		floorBatch.Vertex3f(-20.0f, -0.41f, -20.0f);
	floorBatch.End();

	mirrorBatch.Begin(GL_TRIANGLE_FAN, 4, 1);
		mirrorBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
		mirrorBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
		mirrorBatch.Normal3f( 0.0f, 1.0f, 0.0f);
		mirrorBatch.Vertex3f(-1.0f, 0.0f, 0.0f);

		mirrorBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
		mirrorBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
		mirrorBatch.Normal3f(0.0f, 1.0f, 0.0f);
		mirrorBatch.Vertex3f(1.0f, 0.0f, 0.0f);

		mirrorBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
		mirrorBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
		mirrorBatch.Normal3f(0.0f, 1.0f, 0.0f);
		mirrorBatch.Vertex3f(1.0f, 2.0f, 0.0f);

		mirrorBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
		mirrorBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
		mirrorBatch.Normal3f( 0.0f, 1.0f, 0.0f);
		mirrorBatch.Vertex3f(-1.0f, 2.0f, 0.0f);
	mirrorBatch.End();

	mirrorBorderBatch.Begin(GL_TRIANGLE_STRIP, 13);
		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(-1.0f, 0.1f, 0.01f);

		mirrorBorderBatch.Normal3f(0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(-1.0f, 0.0f, 0.01f);

		mirrorBorderBatch.Normal3f(0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(1.0f, 0.1f, 0.01f);

		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(1.0f, 0.0f, 0.01f);

		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(0.9f, 0.0f, 0.01f);

		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(1.0f, 2.0f, 0.01f);
			
		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(0.9f, 2.0f, 0.01f);
			
		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(1.0f, 1.9f, 0.01f);

		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(-1.0f, 2.f, 0.01f);

		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(-1.0f, 1.9f, 0.01f);

		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(-0.9f, 2.f, 0.01f);

		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(-1.0f, 0.0f, 0.01f);

		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(-0.9f, 0.0f, 0.01f);
	mirrorBorderBatch.End();

	glGenTextures(1, textures);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	LoadBMPTexture("marble.bmp", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);

	// Create and bind an FBO
	glGenFramebuffers(1,&fboName);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboName);

	// Create depth renderbuffer
	glGenRenderbuffers(1, &depthBufferName);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBufferName);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, mirrorTexWidth, mirrorTexHeight);
	 
	// Create the reflection texture
	glGenTextures(1, &mirrorTexture);
	glBindTexture(GL_TEXTURE_2D, mirrorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mirrorTexWidth, mirrorTexHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	
	// Attach texture to first color attachment and the depth RBO
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTexture, 0);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferName);

	// Make sure all went well
	gltCheckErrors();
	
	// Reset framebuffer binding
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}


///////////////////////////////////////////////////////////////////////////////
// Do your cleanup here. Free textures, display lists, buffer objects, etc.
void FBOtextures::Shutdown(void)
{
	// Make sure default FBO is bound
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	// Cleanup textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glDeleteTextures(1, &mirrorTexture);
	glDeleteTextures(1, textures);

	// Cleanup RBOs
	glDeleteRenderbuffers(1, &depthBufferName);

	// Cleanup FBOs
	glDeleteFramebuffers(1, &fboName);

}


///////////////////////////////////////////////////////////////////////////////
// This is called at least once and before any rendering occurs. If the screen
// is a resizeable window, then this will also get called whenever the window
// is resized.
void FBOtextures::Resize(GLsizei nWidth, GLsizei nHeight)
{
	glViewport(0, 0, nWidth, nHeight);
	transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
 
	viewFrustum.SetPerspective(35.0f, float(nWidth)/float(nHeight), 1.0f, 100.0f);
	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	modelViewMatrix.LoadIdentity();

	// update screen sizes
	screenWidth = nWidth;
	screenHeight = nHeight;
}


///////////////////////////////////////////////////////////////////////////////
// Update the camera based on user input, toggle display modes
// 
void FBOtextures::MoveCamera(void)
{ 
	static CStopWatch cameraTimer;
	float fTime = cameraTimer.GetElapsedSeconds();
	cameraTimer.Reset(); 

	float linear = fTime * 3.0f;
	float angular = fTime * float(m3dDegToRad(60.0f));

	if(GetAsyncKeyState(VK_UP))
		cameraFrame.MoveForward(linear);

	if(GetAsyncKeyState(VK_DOWN))
		cameraFrame.MoveForward(-linear);

	if(GetAsyncKeyState(VK_LEFT))
		cameraFrame.RotateWorld(angular, 0.0f, 1.0f, 0.0f);

	if(GetAsyncKeyState(VK_RIGHT))
		cameraFrame.RotateWorld(-angular, 0.0f, 1.0f, 0.0f);
}



///////////////////////////////////////////////////////////////////////////////
// Draw the scene 
// 
void FBOtextures::DrawWorld(GLfloat yRot)
{
	M3DMatrix44f mCamera;
	modelViewMatrix.GetMatrix(mCamera);
	
	// Need light position relative to the Camera
	M3DVector4f vLightTransformed;
	m3dTransformVector4(vLightTransformed, vLightPos, mCamera);

	// Draw the light source as a small white unshaded sphere
	modelViewMatrix.PushMatrix();
		modelViewMatrix.Translatev(vLightPos);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		sphereBatch.Draw();
	modelViewMatrix.PopMatrix();

	// Draw stuff relative to the camera
	modelViewMatrix.PushMatrix();
		modelViewMatrix.Translate(0.0f, 0.2f, -2.5f);
		modelViewMatrix.Rotate(yRot, 0.0f, 1.0f, 0.0f);
	
		shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, 
				modelViewMatrix.GetMatrix(), 
				transformPipeline.GetProjectionMatrix(), 
				vLightTransformed, vGreen, 0);
		torusBatch.Draw();
	modelViewMatrix.PopMatrix();
}


///////////////////////////////////////////////////////////////////////////////
// Render a frame. The owning framework is responsible for buffer swaps,
// flushes, etc.
void FBOtextures::Render(void)
{
	static CStopWatch animationTimer;
	float yRot = animationTimer.GetElapsedSeconds() * 60.0f;
	MoveCamera();

	M3DVector3f vCameraPos;
	M3DVector3f vCameraForward;
	M3DVector3f vMirrorPos;
	M3DVector3f vMirrorForward;
	cameraFrame.GetOrigin(vCameraPos);
	cameraFrame.GetForwardVector(vCameraForward);

	// Set position of mirror frame (camera)
	vMirrorPos[0] = 0.0;
	vMirrorPos[1] = 0.1f;
	vMirrorPos[2] = -6.0f; // view pos is actually behind mirror
	mirrorFrame.SetOrigin(vMirrorPos);

	// Calculate direction of mirror frame (camera)
	// Because the position of the mirror is known relative to the origin
	// find the direction vector by adding the mirror offset to the vector
	// of the viewer-origin
	vMirrorForward[0] = vCameraPos[0];
	vMirrorForward[1] = vCameraPos[1];
	vMirrorForward[2] = (vCameraPos[2] + 5);
	m3dNormalizeVector3(vMirrorForward);
	mirrorFrame.SetForwardVector(vMirrorForward);
	
	// first render from the mirrors perspective
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboName);
	glDrawBuffers(1, fboBuffs);
	glViewport(0, 0, mirrorTexWidth, mirrorTexHeight);

	// Draw scene from the perspective of the mirror camera
	modelViewMatrix.PushMatrix();	
		M3DMatrix44f mMirrorView;
		mirrorFrame.GetCameraMatrix(mMirrorView);
		modelViewMatrix.MultMatrix(mMirrorView);

		// Flip the mirror camera horizontally for the reflection
		modelViewMatrix.Scale(-1.0f, 1.0f, 1.0f);
		
		glBindTexture(GL_TEXTURE_2D, textures[0]); // Marble
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderManager.UseStockShader(GLT_SHADER_TEXTURE_MODULATE, transformPipeline.GetModelViewProjectionMatrix(), vWhite, 0);
		floorBatch.Draw();
		DrawWorld(yRot);

		// Now draw a cylinder representing the viewer
		M3DVector4f vLightTransformed;
		modelViewMatrix.GetMatrix(mMirrorView);
		m3dTransformVector4(vLightTransformed, vLightPos, mMirrorView);
		modelViewMatrix.Translate(vCameraPos[0],vCameraPos[1]-0.8f,vCameraPos[2]-1.0f);	
		modelViewMatrix.Rotate(-90.0f, 1.0f, 0.0f, 0.0f);

		shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, 
					modelViewMatrix.GetMatrix(), 
					transformPipeline.GetProjectionMatrix(), 
					vLightTransformed, vBlue, 0);
		cylinderBatch.Draw();
	modelViewMatrix.PopMatrix();

	// Reset FBO. Draw world again from the real cameras perspective
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffers(1, windowBuff);
	glViewport(0, 0, screenWidth, screenHeight);
	modelViewMatrix.PushMatrix();	
		M3DMatrix44f mCamera;
		cameraFrame.GetCameraMatrix(mCamera);
		modelViewMatrix.MultMatrix(mCamera);
		
		glBindTexture(GL_TEXTURE_2D, textures[0]); // Marble
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderManager.UseStockShader(GLT_SHADER_TEXTURE_MODULATE, transformPipeline.GetModelViewProjectionMatrix(), vWhite, 0);

		floorBatch.Draw();
		DrawWorld(yRot);

		// Now draw the mirror surfaces
		modelViewMatrix.PushMatrix();
			modelViewMatrix.Translate(0.0f, -0.4f, -5.0f);
			if(vCameraPos[2] > -5.0)
			{
				glBindTexture(GL_TEXTURE_2D, mirrorTexture); // Reflection
				shaderManager.UseStockShader(GLT_SHADER_TEXTURE_REPLACE, transformPipeline.GetModelViewProjectionMatrix(), 0);
			}
			else
			{
				// If the camera is behind the mirror, just draw black
				shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlack);
			}
			mirrorBatch.Draw();
			shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGrey);
			mirrorBorderBatch.Draw();
		modelViewMatrix.PopMatrix();
	modelViewMatrix.PopMatrix();
	
}
