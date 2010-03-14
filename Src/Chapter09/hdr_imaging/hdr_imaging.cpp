#include "hdr_imaging.h"
#include <GL\glu.h>
#include <stdio.h>
#include <iostream>
#include <ImfRgbaFile.h>            // OpenEXR headers
#include <ImfArray.h>

#ifdef _WIN32
#pragma comment (lib, "half.lib") 
#pragma comment (lib, "Iex.lib")
#pragma comment (lib, "IlmImf.lib")
#pragma comment (lib, "IlmThread.lib")
#pragma comment (lib, "Imath.lib")
#pragma comment (lib, "zlib.lib")
#endif

#pragma warning (disable : 4305)

////////////////////////////////////////////////////////////////////////////
// Do not put any OpenGL code here. General guidence on constructors in 
// general is to not put anything that can fail here either (opening files,
// allocating memory, etc.)
HDRImaging::HDRImaging(void) : screenWidth(614), screenHeight(655), bFullScreen(false), 
				bAnimated(true), mapTexProg(0), varExposureProg(0), adaptiveProg(0), curHDRTex(0),
				fboName(0), curProg(0), exposure(0.1f)
{
 
}  


////////////////////////////////////////////////////////////////////////////
// Take a file name/location and load an OpenEXR
// Load the image into the "texture" texture object and pass back the texture sizes
// 
bool HDRImaging::LoadOpenEXRImage(char *fileName, GLint textureName, GLuint &texWidth, GLuint &texHeight)
{
	// The OpenEXR uses exception handling to report errors or failures
	// Do all work in a try block to catch any thrown exceptions.
	try
	{
		Imf::Array2D<Imf::Rgba> pixels;
		Imf::RgbaInputFile file (fileName);
		Imath::Box2i dw = file.dataWindow();

		texWidth  = dw.max.x - dw.min.x + 1;
		texHeight = dw.max.y - dw.min.y + 1;
	    
		pixels.resizeErase (texHeight, texWidth); 

		file.setFrameBuffer (&pixels[0][0] - dw.min.x - dw.min.y * texWidth, 1, texWidth);
		file.readPixels (dw.min.y, dw.max.y); 

		GLfloat* texels = (GLfloat*)malloc(texWidth * texHeight * 3 * sizeof(GLfloat));
		GLfloat* pTex = texels;

		// Copy OpenEXR into local buffer for loading into a texture
		for (unsigned int v = 0; v < texHeight; v++)
		{
			for (unsigned int u = 0; u < texWidth; u++)
			{
				Imf::Rgba texel = pixels[texHeight - v - 1][u];  
				pTex[0] = texel.r;
				pTex[1] = texel.g;
				pTex[2] = texel.b;

				pTex += 3;
			}
		}

		// Bind texture, load image, set tex state
		glBindTexture(GL_TEXTURE_2D, textureName);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, texWidth, texHeight, 0, GL_RGB, GL_FLOAT, texels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		
		free(texels);
	}
	catch(Iex::BaseExc & e)  
	{
		std::cerr << e.what() << std::endl;
		//
		// Handle exception.
		//
	}

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// OpenGL related startup code is safe to put here. Load textures, etc.
void HDRImaging::Initialize(void)
{
	GLfloat texCoordOffsets[4][5*5*2];
	GLfloat exposureLUT[20]   = { 11.0, 6.0, 3.2, 2.8, 2.2, 1.90, 1.80, 1.80, 1.70, 1.70,  1.60, 1.60, 1.50, 1.50, 1.40, 1.40, 1.30, 1.20, 1.10, 1.00 };
	
	// Make sure OpenGL entry points are set
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}

	// Will not use depth buffer
	glDisable(GL_DEPTH_TEST);
	curHDRTex = 0;
	
	// Init codel-view and leave it 
	modelViewMatrix.LoadIdentity();

	// Black
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Setup LUT texture for use with the adaptive exposure filter
	glGenTextures(1, lutTxtures);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, lutTxtures[1]);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_R16F, 20,  0, GL_RED, GL_FLOAT, exposureLUT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Setup HDR texture(s)
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, hdrTextures);
	glBindTexture(GL_TEXTURE_2D, hdrTextures[curHDRTex]);

	// Load HDR image from EXR file
    LoadOpenEXRImage("Tree.exr", hdrTextures[curHDRTex], hdrTexturesWidth[curHDRTex], hdrTexturesHeight[curHDRTex]);

	// Create ortho matrix and screen-sized quad matching images aspect ratio
    GenerateOrtho2DMat(GetWidth(), GetHeight(), hdrTexturesWidth[curHDRTex], hdrTexturesHeight[curHDRTex]);
	//GenerateFBOOrtho2DMat(hdrTexturesWidth[curHDRTex], hdrTexturesHeight[curHDRTex]);
    gltGenerateOrtho2DMat(hdrTexturesWidth[curHDRTex], hdrTexturesHeight[curHDRTex], fboOrthoMatrix, fboQuad);

	// Setup tex coords to be used for fetching HDR kernel data
	for (int k = 0; k < 4; k++)
	{
		float xInc = 1.0f / (GLfloat)(hdrTexturesWidth[curHDRTex] >> k);
		float yInc = 1.0f / (GLfloat)(hdrTexturesHeight[curHDRTex] >> k);

		for (int i = 0; i < 5; i++)
		{
			for (int j = 0; j < 5; j++)
			{
				texCoordOffsets[k][(((i*5)+j)*2)+0] = (-1.0f * xInc) + ((GLfloat)i * xInc);
				texCoordOffsets[k][(((i*5)+j)*2)+1] = (-1.0f * yInc) + ((GLfloat)j * yInc);
			}
		}
	}

	// Load shaders 
    mapTexProg =  gltLoadShaderPairWithAttributes("hdr.vs", "hdr_simple.fs", 2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");
	glBindFragDataLocation(mapTexProg, 0, "oColor");
	glLinkProgram(mapTexProg);

	varExposureProg =  gltLoadShaderPairWithAttributes("hdr.vs", "hdr_exposure.fs", 2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");
	glBindFragDataLocation(varExposureProg, 0, "oColor");
	glLinkProgram(varExposureProg);
	glUseProgram(varExposureProg);
	glUniform1i(glGetUniformLocation(varExposureProg, "textureUnit0"), 0);

	adaptiveProg =  gltLoadShaderPairWithAttributes("hdr.vs", "hdr_adaptive.fs", 2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");
	glBindFragDataLocation(adaptiveProg, 0, "oColor");
	glLinkProgram(adaptiveProg);
	glUseProgram(adaptiveProg);
	glUniform1i(glGetUniformLocation(adaptiveProg, "textureUnit0"), 0);
	glUniform1i(glGetUniformLocation(adaptiveProg, "textureUnit1"), 1);
	glUniform2fv(glGetUniformLocation(adaptiveProg, "tc_offset"), 25, texCoordOffsets[0]);

	glUseProgram(0);

	// Create and bind an FBO
	glGenFramebuffers(1,&fboName);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboName);

	// Create the FBO texture
	glGenTextures(1, fboTextures);
	glBindTexture(GL_TEXTURE_2D, fboTextures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, hdrTexturesWidth[curHDRTex], hdrTexturesHeight[curHDRTex], 0, GL_RGBA, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTextures[0], 0);
    
	// Make sure all went well
	gltCheckErrors();
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	// Set first running mode
	curProg = adaptiveProg;
}


///////////////////////////////////////////////////////////////////////////////
// Do your cleanup here. Free textures, display lists, buffer objects, etc.
void HDRImaging::Shutdown(void)
{
	// Make sure default FBO is bound
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	glDeleteFramebuffers(1, &fboName);

	// Cleanup textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, 0);
	
	glDeleteTextures(1, hdrTextures);
	glDeleteTextures(1, lutTxtures);
	glDeleteTextures(1, fboTextures);

}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Create a matrix that maps geometry to the screen. 1 unit in the x direction equals one pixel 
// of width, same with the y direction.
// It also depends on the size of the texture being displayed
void HDRImaging::GenerateOrtho2DMat(GLuint windowWidth, GLuint windowHeight, GLuint imageWidth, GLuint imageHeight)
{
    float right = (float)windowWidth;
	float quadWidth = right;
	float left  = 0.0f;
	float top = (float)windowHeight;
	float quadHeight = top;
	float bottom = 0.0f;
	float screenAspect = (float)windowWidth/windowHeight;
	float imageAspect = (float)imageWidth/imageHeight;

	if (screenAspect > imageAspect)
		quadWidth = windowHeight*imageAspect;
	else
		quadHeight = windowWidth*imageAspect;

    // set ortho matrix
	orthoMatrix[0] = (float)(2 / (right - left));
	orthoMatrix[1] = 0.0;
	orthoMatrix[2] = 0.0;
	orthoMatrix[3] = 0.0;

	orthoMatrix[4] = 0.0;
	orthoMatrix[5] = (float)(2 / (top - bottom));
	orthoMatrix[6] = 0.0;
	orthoMatrix[7] = 0.0;

	orthoMatrix[8] = 0.0;
	orthoMatrix[9] = 0.0;
	orthoMatrix[10] = (float)(-2 / (1.0 - 0.0));
	orthoMatrix[11] = 0.0;

	orthoMatrix[12] = -1*(right + left) / (right - left);
	orthoMatrix[13] = -1*(top + bottom) / (top - bottom);
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
		screenQuad.Vertex3f(quadWidth, 0.0f, 0.0f);

		screenQuad.Color4f(0.0f, 1.0f, 0.0f, 1.0f);
		screenQuad.MultiTexCoord2f(0, 0.0f, 1.0f);
		screenQuad.Vertex3f(0.0f, quadHeight, 0.0f);

		screenQuad.Color4f(0.0f, 1.0f, 0.0f, 1.0f);
		screenQuad.MultiTexCoord2f(0, 1.0f, 1.0f);
		screenQuad.Vertex3f(quadWidth, quadHeight, 0.0f);
	screenQuad.End();

}



///////////////////////////////////////////////////////////////////////////////
// This is called at least once and before any rendering occurs. If the screen
// is a resizeable window, then this will also get called whenever the window
// is resized.
void HDRImaging::Resize(GLsizei nWidth, GLsizei nHeight)
{
	glViewport(0, 0, nWidth, nHeight);
	transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
 
	modelViewMatrix.LoadIdentity();

	// update screen sizes
	screenWidth = nWidth;
	screenHeight = nHeight;

    GenerateOrtho2DMat(nWidth, nHeight, hdrTexturesWidth[curHDRTex], hdrTexturesHeight[curHDRTex]);
}


///////////////////////////////////////////////////////////////////////////////
// Update the camera based on user input, toggle display modes
// 
void HDRImaging::UpdateMode(void)
{ 
	static CStopWatch timer;
	float fTime = timer.GetElapsedSeconds();
	float linear = fTime / 100;

	if(GetAsyncKeyState(VK_UP))
	{
		if((exposure + linear) < 25.0f)
			exposure += linear;
	}
	if(GetAsyncKeyState(VK_DOWN))
	{
		if((exposure - linear) > 0.01f)
			exposure -= linear;
	}

	if(GetAsyncKeyState('1'))
	{
		curProg = mapTexProg;
	}
	if(GetAsyncKeyState('2'))
	{
		curProg = varExposureProg;
	}
	if(GetAsyncKeyState('3'))
	{
		curProg = adaptiveProg;
	}
}

void HDRImaging::SetupStraightTexProg()
{
	// Set the cur prog for tex replace
	glUseProgram(mapTexProg);

	// Set MVP matrix
	glUniformMatrix4fv(glGetUniformLocation(mapTexProg, "mvpMatrix"), 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
}

void HDRImaging::SetupHDRProg()
{
    // Set the program to the cur
	glUseProgram(curProg);

	// Set MVP matrix
	glUniformMatrix4fv(glGetUniformLocation(curProg, "mvpMatrix"), 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());

	if (curProg == varExposureProg)
	{
		// Set the exposure for the scene
		glUniform1f(glGetUniformLocation(curProg, "exposure"), exposure);
	}	
}

///////////////////////////////////////////////////////////////////////////////
// Render a frame. The owning framework is responsible for buffer swaps,
// flushes, etc.
void HDRImaging::Render(void)
{
	static CStopWatch animationTimer;
	float yRot = animationTimer.GetElapsedSeconds() * 60.0f;
	UpdateMode();

	// first, draw to FBO at full FBO resolution

	// Bind FBO with 8b attachment
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboName);
	glViewport(0, 0, hdrTexturesWidth[curHDRTex], hdrTexturesHeight[curHDRTex]);
	glClear(GL_COLOR_BUFFER_BIT);

	// Bind texture with HDR image 
	glBindTexture(GL_TEXTURE_2D, hdrTextures[curHDRTex]);

	// Render pass, downsample to 8b using selected program
	projectionMatrix.LoadMatrix(fboOrthoMatrix);
	SetupHDRProg();
	fboQuad.Draw();

	// Then draw the resulting image to the screen, maintain image proportions 
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glViewport(0, 0, screenWidth, screenHeight);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	// Attach 8b texture with HDR image 
	glBindTexture(GL_TEXTURE_2D, fboTextures[0]);
	
	// draw screen sized, proportional quad with 8b texture
	projectionMatrix.LoadMatrix(orthoMatrix);
	SetupStraightTexProg();
	screenQuad.Draw();

}
