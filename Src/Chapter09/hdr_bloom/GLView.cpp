// GLView.cpp
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

#include "GLView.h"
#include <GL\glu.h>
#include <stdio.h>
#include <iostream>

static GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };
static GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
static GLfloat vBlue[] = { 0.0f, 0.0f, 1.0f, 1.0f };
static GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static GLfloat vLightPos[] = { 0.0f, 3.0f, 0.0f, 1.0f };
static const GLenum windowFboBuff[] = { GL_BACK_LEFT };
static const GLenum fboBuff[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };


static float* LoadFloatData(const char *szFile, int *count)
{
	GLint lineCount = 0;
	FILE *fp;
	float* data = 0;
	
    // Open the shader file
    fp = fopen(szFile, "r");
    if(fp != NULL)
	{
		char szFloat[1024];
        while ( fgets ( szFloat, sizeof szFloat, fp ) != NULL )
            lineCount++;
		
        // Go back to beginning of file
        rewind(fp);
		
        // Allocate space for all data
		data = (float*)malloc((lineCount)*sizeof(float));
		if (data != NULL)
		{	
			int index = 0;
			while ( fgets ( szFloat, sizeof szFloat, fp ) != NULL )
			{
				data[index] = (float)atof(szFloat);
				index++;
			}
			count[0] = index;
		}
        fclose(fp);
	}
    else
        return 0;    
    
    return data;
}

////////////////////////////////////////////////////////////////////////////
// Do not put any OpenGL code here. General guidence on constructors in 
// general is to not put anything that can fail here either (opening files,
// allocating memory, etc.)
GLView::GLView(void) : screenWidth(800), screenHeight(600), bFullScreen(false), 
				bAnimated(true), bUseFBO(true), fboName(0), depthBufferName(0)
{

}
	
void CheckErrors(GLuint progName = 0)
{
	GLenum error = glGetError();
	GLenum fboError = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);

	if (error != GL_NO_ERROR)
	{
		cout << "A GL Error has occured\n";
	}
	if(fboError != GL_FRAMEBUFFER_COMPLETE)
	{
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

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Load in a BMP file as a texture. Allows specification of the filters and the wrap mode
bool GLView::LoadBMPTexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)	
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
void GLView::Initialize(void)
{
	// Initialze Shader Manager
	shaderManager.InitializeStockShaders();

	glEnable(GL_DEPTH_TEST);

	// Black
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	gltMakeTorus(torusBatch, 0.4f, 0.15f, 35, 35);
	gltMakeSphere(sphereBatch, 0.1f, 26, 13);

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

	glGenTextures(3, textures);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	LoadBMPTexture("marble.bmp", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);

	glGenFramebuffers(1,&fboName);

	// Create depth buffer
	glGenRenderbuffers(1, &depthBufferName);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBufferName);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, screenWidth, screenHeight);
	
	glGenRenderbuffers(3, renderBufferNames);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBufferNames[0]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, screenWidth, screenHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBufferNames[1]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, screenWidth, screenHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBufferNames[2]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, screenWidth, screenHeight);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboName);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferName);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderBufferNames[0]);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, renderBufferNames[1]);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_RENDERBUFFER, renderBufferNames[2]);

	// Setup shader for processing
	processProg =  gltLoadShaderPairWithAttributes("multibuffer.vs", "multibuffer.fs",3,
																GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_NORMAL, "vNormal", GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");
    
	// Create new buffer objects
	glGenBuffers(3,texBO);
	glGenTextures(1, &texBOTexture);
	
	glBindBuffer(GL_TEXTURE_BUFFER_ARB, texBO[0]);

	int count = 0;
	float* fileData = 0;
	
	fileData = LoadFloatData("LumTan.data", &count);
	if (count > 0)
	{
		glBindBuffer(GL_TEXTURE_BUFFER_ARB, texBO[0]);
		glBufferData(GL_TEXTURE_BUFFER_ARB, sizeof(float)*count, fileData, GL_STATIC_DRAW);
		delete fileData;
	}

	fileData = LoadFloatData("LumSin.data", &count);
	if (count > 0)
	{
		glBindBuffer(GL_TEXTURE_BUFFER_ARB, texBO[1]);
		glBufferData(GL_TEXTURE_BUFFER_ARB, sizeof(float)*count, fileData, GL_STATIC_DRAW);
		delete fileData;
	}

	fileData = LoadFloatData("LumLinear.data", &count);
	if (count > 0)
	{
		glBindBuffer(GL_TEXTURE_BUFFER_ARB, texBO[2]);
		glBufferData(GL_TEXTURE_BUFFER_ARB, sizeof(float)*count, fileData, GL_STATIC_DRAW);
		delete fileData;
	}

	// Load the Tan ramp first
	glBindBuffer(GL_TEXTURE_BUFFER_ARB, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_BUFFER_ARB, texBOTexture);
	glTexBufferARB(GL_TEXTURE_BUFFER_ARB, GL_R32F, texBO[0]); // FIX THIS IN GLEE
	glActiveTexture(GL_TEXTURE0);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	CheckErrors();
}


///////////////////////////////////////////////////////////////////////////////
// Do your cleanup here. Free textures, display lists, buffer objects, etc.
void GLView::Shutdown(void)
{


}


///////////////////////////////////////////////////////////////////////////////
// This is called at least once and before any rendering occurs. If the screen
// is a resizeable window, then this will also get called whenever the window
// is resized.
void GLView::Resize(GLsizei nWidth, GLsizei nHeight)
{
	glViewport(0, 0, nWidth, nHeight);
	transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
 
	viewFrustum.SetPerspective(35.0f, float(nWidth)/float(nHeight), 1.0f, 100.0f);
	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	modelViewMatrix.LoadIdentity();

	// update screen sizes
	screenWidth = nWidth;
	screenHeight = nHeight;

	glBindRenderbuffer(GL_RENDERBUFFER, depthBufferName);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, GetWidth(), GetHeight());
	glBindRenderbuffer(GL_RENDERBUFFER, renderBufferNames[0]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, screenWidth, screenHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBufferNames[1]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, screenWidth, screenHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBufferNames[2]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, screenWidth, screenHeight);
}


///////////////////////////////////////////////////////////////////////////////
// Update the camera based on user input, toggle display modes
// 
void GLView::MoveCamera(void)
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

	static bool bF2IsDown = false;
	if(GetAsyncKeyState(VK_F2))
	{
		if(bF2IsDown == false)
		{
			bF2IsDown = true;
			bUseFBO = !bUseFBO;
		}
	}
	else
	{
		bF2IsDown = false; 
	}

	if(GetAsyncKeyState(VK_F3))
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_BUFFER_ARB, texBOTexture);
		glTexBufferARB(GL_TEXTURE_BUFFER_ARB, GL_R32F, texBO[0]); // FIX THIS IN GLEE
		glActiveTexture(GL_TEXTURE0);
	}
	else if(GetAsyncKeyState(VK_F4))
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_BUFFER_ARB, texBOTexture);
		glTexBufferARB(GL_TEXTURE_BUFFER_ARB, GL_R32F, texBO[1]); // FIX THIS IN GLEE
		glActiveTexture(GL_TEXTURE0);
	}
	else if(GetAsyncKeyState(VK_F5))
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_BUFFER_ARB, texBOTexture);
		glTexBufferARB(GL_TEXTURE_BUFFER_ARB, GL_R32F, texBO[2]); // FIX THIS IN GLEE
		glActiveTexture(GL_TEXTURE0);
	}
}


///////////////////////////////////////////////////////////////////////////////
// Enable and setup the GLSL program used for 
// flushes, etc.
void GLView::UseProcessProgram(M3DVector4f vLightPos, M3DVector4f vColor, int textureUnit)
{
	int location = 0;

	glUseProgram(processProg);

	glUniformMatrix4fv(glGetUniformLocation(processProg, "mvMatrix"), 
				1, GL_FALSE, transformPipeline.GetModelViewMatrix());

	glUniformMatrix4fv(glGetUniformLocation(processProg, "pMatrix"), 
				1, GL_FALSE, transformPipeline.GetProjectionMatrix());

	glUniform3fv(glGetUniformLocation(processProg, "vLightPos"), 1, vLightPos);

	glUniform4fv(glGetUniformLocation(processProg, "vColor"), 1, vColor);
	glUniform1i(glGetUniformLocation(processProg, "lumCurveSampler"), 1);

	if(textureUnit != -1)
	{
		glUniform1i(glGetUniformLocation(processProg, "bUseTexture"), 1);
		glUniform1i(glGetUniformLocation(processProg, "textureUnit0"), textureUnit);
	}
	else
	{
		glUniform1i(glGetUniformLocation(processProg, "bUseTexture"), 0);
	}

	CheckErrors(processProg);
}

///////////////////////////////////////////////////////////////////////////////
// Draw the scene 
// 
void GLView::DrawWorld(GLfloat yRot)
{
	M3DMatrix44f mCamera;
	modelViewMatrix.GetMatrix(mCamera);
	
	// Need light position relative to the Camera
	M3DVector4f vLightTransformed;
	m3dTransformVector4(vLightTransformed, vLightPos, mCamera);

	// Draw the light source as a small white unshaded sphere
	modelViewMatrix.PushMatrix();
		modelViewMatrix.Translatev(vLightPos);

		if(bUseFBO)
			UseProcessProgram(vLightPos, vWhite, -1);
		else
			shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);

		sphereBatch.Draw();
	modelViewMatrix.PopMatrix();

	// Draw stuff relative to the camera
	modelViewMatrix.PushMatrix();
		modelViewMatrix.Translate(0.0f, 0.2f, -2.5f);

		modelViewMatrix.PushMatrix();
			modelViewMatrix.Rotate(yRot, 0.0f, 1.0f, 0.0f);

			if(bUseFBO)
			{
				UseProcessProgram(vLightTransformed, vGreen, -1);
			}
			else
			{
				shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, 
					modelViewMatrix.GetMatrix(), 
					transformPipeline.GetProjectionMatrix(), 
					vLightTransformed, vGreen, 0);
			}
			torusBatch.Draw();
			modelViewMatrix.PopMatrix();

	modelViewMatrix.PopMatrix();
}


///////////////////////////////////////////////////////////////////////////////
// Render a frame. The owning framework is responsible for buffer swaps,
// flushes, etc.
void GLView::Render(void)
{
	static CStopWatch animationTimer;
	float yRot = animationTimer.GetElapsedSeconds() * 60.0f;
	MoveCamera();

	modelViewMatrix.PushMatrix();	
		M3DMatrix44f mCamera;
		cameraFrame.GetCameraMatrix(mCamera);
		modelViewMatrix.MultMatrix(mCamera);
		
		GLfloat vFloorColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

		glBindTexture(GL_TEXTURE_2D, textures[0]); // Marble

		if(bUseFBO)
		{
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboName);
			glDrawBuffers(3, fboBuff);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Need light position relative to the Camera
			M3DVector4f vLightTransformed;
			m3dTransformVector4(vLightTransformed, vLightPos, mCamera);
			UseProcessProgram(vLightTransformed, vFloorColor, 0);
		}
		else
		{
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glDrawBuffers(1, windowFboBuff);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			shaderManager.UseStockShader(GLT_SHADER_TEXTURE_MODULATE, transformPipeline.GetModelViewProjectionMatrix(), vFloorColor, 0);
		}

		floorBatch.Draw();
		DrawWorld(yRot);

	modelViewMatrix.PopMatrix();

	if(bUseFBO)
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glDrawBuffers(1, windowFboBuff);
		glViewport(0, 0, GetWidth(), GetHeight());

		glBindFramebuffer(GL_READ_FRAMEBUFFER, fboName);

		glReadBuffer(GL_COLOR_ATTACHMENT1);
		glBlitFramebuffer(0, 0, GetWidth()/2, GetHeight(),
						  0, 0, GetWidth()/2, GetHeight(),
						  GL_COLOR_BUFFER_BIT, GL_NEAREST );
	
		glReadBuffer(GL_COLOR_ATTACHMENT2);	
		glBlitFramebuffer(GetWidth()/2, 0, GetWidth(), GetHeight(),
						  GetWidth()/2, 0, GetWidth(), GetHeight(),
						  GL_COLOR_BUFFER_BIT, GL_NEAREST );

		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glBlitFramebuffer(0, 0, GetWidth(), GetHeight(),
						  (int)(GetWidth() *(0.8)), (int)(GetHeight()*(0.8)), 
						  GetWidth(), GetHeight(),
						  GL_COLOR_BUFFER_BIT, GL_LINEAR );

		glBindTexture(GL_TEXTURE_2D, 0);
	} 
}
