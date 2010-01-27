#include "hdr_bloom.h"
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

#pragma warning( disable : 4244)

static GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };
static GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
static GLfloat vBlue[] = { 0.0f, 0.0f, 1.0f, 1.0f };
static GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static GLfloat vWhiteX2[] = { 2.0f, 2.0f, 2.0f, 2.0f };
static GLfloat vBlack[] = { 0.0f, 0.0f, 0.0f, 1.0f };
static GLfloat vGrey[] =  { 0.5f, 0.5f, 0.5f, 1.0f };
static GLfloat vLightPos[] = { -2.0f, 3.0f, -2.0f, 1.0f };
static GLfloat vSkyBlue[] = { 0.160f, 0.376f, 0.925f, 1.0f};

static const GLenum windowBuff[] = { GL_BACK_LEFT };
static const GLenum fboBuffs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };


////////////////////////////////////////////////////////////////////////////
// Do not put any OpenGL code here. General guidence on constructors in 
// general is to not put anything that can fail here either (opening files,
// allocating memory, etc.)
HDRBloom::HDRBloom(void) : screenWidth(800), screenHeight(600), bFullScreen(false), 
				bAnimated(true), bloomLevel(0.0)
{
	cameraFrame.MoveUp(0.50);

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

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Load in a BMP file as a texture. Allows specification of the filters and the wrap mode
bool HDRBloom::LoadBMPTexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)	
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



void HDRBloom::GenTexCoordOffsets(GLuint width, GLuint height)
{
	float xInc = 1.0f / (GLfloat)(width);
	float yInc = 1.0f / (GLfloat)(height);

	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			texCoordOffsets[(((i*5)+j)*2)+0] = (-2.0f * xInc) + ((GLfloat)i * xInc);
			texCoordOffsets[(((i*5)+j)*2)+1] = (-2.0f * yInc) + ((GLfloat)j * yInc);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// OpenGL related startup code is safe to put here. Load textures, etc.
void HDRBloom::Initialize(void)
{
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}

    glEnable(GL_DEPTH_TEST);

	exposure = 1.0f;
	bloomLevel = 0.5;

	// Light Blue
	glClearColor(vSkyBlue[0], vSkyBlue[1], vSkyBlue[2], vSkyBlue[3]);

	// Load geometry
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

	windowBatch.Begin(GL_TRIANGLE_FAN, 4, 1);
		windowBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
		windowBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
		windowBatch.Normal3f( 0.0f, 1.0f, 0.0f);
		windowBatch.Vertex3f(-1.0f, 0.0f, 0.0f);

		windowBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
		windowBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
		windowBatch.Normal3f(0.0f, 1.0f, 0.0f);
		windowBatch.Vertex3f(1.0f, 0.0f, 0.0f);

		windowBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
		windowBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
		windowBatch.Normal3f(0.0f, 1.0f, 0.0f);
		windowBatch.Vertex3f(1.0f, 2.0f, 0.0f);

		windowBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
		windowBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
		windowBatch.Normal3f( 0.0f, 1.0f, 0.0f);
		windowBatch.Vertex3f(-1.0f, 2.0f, 0.0f);
	windowBatch.End();

	const float width = 0.2f;
	const float gridWidth = (float)0.05;
	windowBorderBatch.Begin(GL_TRIANGLE_STRIP, 13);
		windowBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		windowBorderBatch.Vertex3f(-1.01f, width, 0.01f);

		windowBorderBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowBorderBatch.Vertex3f(-1.01f, 0.0f, 0.01f);

		windowBorderBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowBorderBatch.Vertex3f(1.01f, width, 0.01f);

		windowBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		windowBorderBatch.Vertex3f(1.01f, 0.0f, 0.01f);

		windowBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		windowBorderBatch.Vertex3f(1.01-width, 0.0f, 0.01f);

		windowBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		windowBorderBatch.Vertex3f(1.01f, 2.0f, 0.01f);
			
		windowBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		windowBorderBatch.Vertex3f(1.01-width, 2.0f, 0.01f);
			
		windowBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		windowBorderBatch.Vertex3f(1.01f, 2.0-width, 0.01f);

		windowBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		windowBorderBatch.Vertex3f(-1.01f, 2.f, 0.01f);

		windowBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		windowBorderBatch.Vertex3f(-1.01f, 2.0-width, 0.01f);

		windowBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		windowBorderBatch.Vertex3f(-1.01+width, 2.f, 0.01f);

		windowBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		windowBorderBatch.Vertex3f(-1.01f, 0.0f, 0.01f);

		windowBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		windowBorderBatch.Vertex3f(-1.01+width, 0.0f, 0.01f);
	windowBorderBatch.End();

	windowGridBatch.Begin(GL_TRIANGLES, 24);
		// bottom horizontal
		windowGridBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(-1.0f, 0.7+gridWidth, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(-1.0f, 0.7-gridWidth, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(1.0f, 0.7-gridWidth, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(1.0f, 0.7-gridWidth, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(1.0f, 0.7+gridWidth, 0.01f);

		windowGridBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(-1.0f, 0.7+gridWidth, 0.01f);
		
		// Top horizontal
		windowGridBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(-1.0f, 1.3+gridWidth, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(-1.0f, 1.3-gridWidth, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(1.0f, 1.3-gridWidth, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(1.0f, 1.3-gridWidth, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(1.0f, 1.3+gridWidth, 0.01f);

		windowGridBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(-1.0f, 1.3+gridWidth, 0.01f);
		
		// Left Vertical
		windowGridBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(-0.3+gridWidth, 0.0f, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(-0.3-gridWidth, 0.0f, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(-0.3-gridWidth, 2.0f, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(-0.3-gridWidth, 2.0f, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(-0.3+gridWidth, 2.0, 0.01f);

		windowGridBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(-0.3+gridWidth, 0.0f, 0.01f);

		// Right Vertical
		windowGridBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(0.3+gridWidth, 0.0f, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(0.3-gridWidth, 0.0f, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(0.3-gridWidth, 2.0f, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(0.3-gridWidth, 2.0f, 0.01f);

		windowGridBatch.Normal3f(0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(0.3+gridWidth, 2.0, 0.01f);

		windowGridBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		windowGridBatch.Vertex3f(0.3+gridWidth, 0.0f, 0.01f);
	windowGridBatch.End();

	glGenTextures(1, textures);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	LoadBMPTexture("marble.bmp", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);

	// Setup HDR render texture
	glGenTextures(1, hdrTextures);
	glBindTexture(GL_TEXTURE_2D, hdrTextures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		
	// create textures for bloom effect
	glGenTextures(5, brightBlurTextures);
	int i = 0;
	for (i=0; i<5; i++)
	{
		glBindTexture(GL_TEXTURE_2D, brightBlurTextures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}

	// Attach HDR texture to fbo
	// Create and bind an FBO
	glGenFramebuffers(1,hdrFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdrFBO[0]);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdrTextures[0], 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, brightBlurTextures[0], 0);

	// Create FBOs for bloom effect
	glGenFramebuffers(4,brightPassFBO);
	for (i=0; i<4; i++)
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, brightPassFBO[i]);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brightBlurTextures[i+1], 0);
		CheckErrors();
	}

	// Create window texture
	glGenTextures(1, &windowTexture);
	glBindTexture(GL_TEXTURE_2D, windowTexture);
	GLuint texWidth = 0;
	GLuint texHeight = 0;
	// Load HDR image from EXR file
    LoadOpenEXRImage("window.exr", windowTexture, texWidth, texHeight);

	// Load flat color shader
	flatColorProg =  gltLoadShaderPairWithAttributes("basic.vs", "color.fs", 3, 
							GLT_ATTRIBUTE_VERTEX, "vVertex", 
							GLT_ATTRIBUTE_NORMAL, "vNormal", 
							GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");
	glBindFragDataLocation(flatColorProg, 0, "oColor");
	glBindFragDataLocation(flatColorProg, 1, "oBright");
	glLinkProgram(flatColorProg);

	// Load texture replace shader
	texReplaceProg =  gltLoadShaderPairWithAttributes("basic.vs", "tex_replace.fs", 3, 
							GLT_ATTRIBUTE_VERTEX, "vVertex", 
							GLT_ATTRIBUTE_NORMAL, "vNormal", 
							GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");
	glBindFragDataLocation(texReplaceProg, 0, "oColor");
	glBindFragDataLocation(texReplaceProg, 1, "oBright");
	glLinkProgram(texReplaceProg);

	// Load bloom shader
	hdrBloomProg =  gltLoadShaderPairWithAttributes("basic.vs", "hdr_exposure.fs", 3, 
							GLT_ATTRIBUTE_VERTEX, "vVertex", 
							GLT_ATTRIBUTE_NORMAL, "vNormal", 
							GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");
	glBindFragDataLocation(hdrBloomProg, 0, "oColor");
	glLinkProgram(hdrBloomProg);

	// Load blur shader
	blurProg =  gltLoadShaderPairWithAttributes("basic.vs", "blur.fs", 2,
							GLT_ATTRIBUTE_VERTEX, "vVertex", 
							GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");
	glBindFragDataLocation(blurProg, 0, "oColor");
	glLinkProgram(blurProg);
	glUseProgram(blurProg);
	
	// Setup tex coords to be used for fetching HDR kernel data
	GenTexCoordOffsets(screenWidth, screenHeight);
	glUniform2fv(glGetUniformLocation(blurProg, "tc_offset"), 25, &texCoordOffsets[0]);

	// Make sure all went well
	CheckErrors(flatColorProg);
	CheckErrors(texReplaceProg);
	CheckErrors(hdrBloomProg);
	CheckErrors(blurProg);
	
	// Reset framebuffer binding
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

////////////////////////////////////////////////////////////////////////////
// Take a file name/location and load an OpenEXR
// Load the image into the "texture" texture object and pass back the texture sizes
// 
bool HDRBloom::LoadOpenEXRImage(char *fileName, GLint textureName, GLuint &texWidth, GLuint &texHeight)
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
// Do your cleanup here. Free textures, display lists, buffer objects, etc.
void HDRBloom::Shutdown(void)
{
	// Make sure default FBO is bound
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	// Cleanup textures
	for (int i=0; i<5;i++)
	{
		glActiveTexture(GL_TEXTURE0+i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glDeleteTextures(1, textures);
	glDeleteTextures(1, hdrTextures);
	glDeleteTextures(5, brightBlurTextures);
	glDeleteTextures(1, &windowTexture);

	// Cleanup FBOs
	glDeleteFramebuffers(1, hdrFBO);
	glDeleteFramebuffers(1, brightPassFBO);

}
void HDRBloom::SetupFlatColorProg(GLfloat *vLightPos, GLfloat *vColor)
{
	glUseProgram(flatColorProg);

	// Set projection matrix
	glUniformMatrix4fv(glGetUniformLocation(flatColorProg, "pMatrix"), 
		1, GL_FALSE, transformPipeline.GetProjectionMatrix());

	// Set MVP matrix
	glUniformMatrix4fv(glGetUniformLocation(flatColorProg, "mvMatrix"), 
		1, GL_FALSE, transformPipeline.GetModelViewMatrix());

	// Set Light Pos
	glUniform3fv(glGetUniformLocation(flatColorProg, "vLightPos"), 1, vLightPos);

	// Set Color
	glUniform4fv(glGetUniformLocation(flatColorProg, "vColor"), 1, vColor);

	CheckErrors(flatColorProg);
}

void HDRBloom::SetupTexReplaceProg(GLfloat *vLightPos, GLfloat *vColor)
{
	glUseProgram(texReplaceProg);

	// Set projection matrix
	glUniformMatrix4fv(glGetUniformLocation(texReplaceProg, "pMatrix"), 
		1, GL_FALSE, transformPipeline.GetProjectionMatrix());

	// Set MVP matrix
	glUniformMatrix4fv(glGetUniformLocation(texReplaceProg, "mvMatrix"), 
		1, GL_FALSE, transformPipeline.GetModelViewMatrix());

	// Set Light Pos
	glUniform3fv(glGetUniformLocation(texReplaceProg, "vLightPos"), 1, vLightPos);
	
	// Set Color
	glUniform4fv(glGetUniformLocation(texReplaceProg, "vColor"), 1, vColor);

	// Set Tex Unit
	glUniform1i(glGetUniformLocation(texReplaceProg, "textureUnit0"), 0);

	CheckErrors(texReplaceProg);

}

void HDRBloom::SetupHDRProg()
{
	glUseProgram(hdrBloomProg);

	// Set projection matrix
	glUniformMatrix4fv(glGetUniformLocation(hdrBloomProg, "pMatrix"), 
		1, GL_FALSE, transformPipeline.GetProjectionMatrix());

	// Set MVP matrix
	glUniformMatrix4fv(glGetUniformLocation(hdrBloomProg, "mvMatrix"), 
		1, GL_FALSE, transformPipeline.GetModelViewMatrix());

	// Set user controled uniforms
	glUniform1fv(glGetUniformLocation(hdrBloomProg, "exposure"), 1, &exposure);
	glUniform1fv(glGetUniformLocation(hdrBloomProg, "bloomLevel"), 1, &bloomLevel);

	// Set texture uniforms
	glUniform1i(glGetUniformLocation(hdrBloomProg, "origImage"), 0);
	glUniform1i(glGetUniformLocation(hdrBloomProg, "brightImage"), 1);
	glUniform1i(glGetUniformLocation(hdrBloomProg, "blur1"), 2);
	glUniform1i(glGetUniformLocation(hdrBloomProg, "blur2"), 3);
	glUniform1i(glGetUniformLocation(hdrBloomProg, "blur3"), 4);
	glUniform1i(glGetUniformLocation(hdrBloomProg, "blur4"), 5);

	// Now setup the right textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTextures[0]);
	for (int i=0; i<5; i++)
	{
		glActiveTexture(GL_TEXTURE1+i);
		glBindTexture(GL_TEXTURE_2D, brightBlurTextures[i]);
	}
	CheckErrors(hdrBloomProg);
} 

void HDRBloom::SetupBlurProg()
{
    // Set the program to the cur
	glUseProgram(blurProg);

	// Set projection matrix
	glUniformMatrix4fv(glGetUniformLocation(blurProg, "pMatrix"), 
		1, GL_FALSE, transformPipeline.GetProjectionMatrix());

	// Set MVP matrix
	glUniformMatrix4fv(glGetUniformLocation(blurProg, "mvMatrix"), 
		1, GL_FALSE, transformPipeline.GetModelViewMatrix());

	glUniform1i(glGetUniformLocation(blurProg, "textureUnit0"), 0);

	CheckErrors(blurProg);

}
///////////////////////////////////////////////////////////////////////////////
// This is called at least once and before any rendering occurs. If the screen
// is a resizeable window, then this will also get called whenever the window
// is resized.
void HDRBloom::Resize(GLsizei nWidth, GLsizei nHeight)
{
	glViewport(0, 0, nWidth, nHeight);
	transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
 
	viewFrustum.SetPerspective(35.0f, float(nWidth)/float(nHeight), 1.0f, 100.0f);
	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	modelViewMatrix.LoadIdentity();

	// update screen sizes
	screenWidth = nWidth;
	screenHeight = nHeight;

	glBindTexture(GL_TEXTURE_2D, hdrTextures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glBindTexture(GL_TEXTURE_2D, brightBlurTextures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);

	for(int i=1; i<5; i++)
	{	
		glBindTexture(GL_TEXTURE_2D, brightBlurTextures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth/(i*3.0), screenHeight/(i*3.0), 0, GL_RGBA, GL_FLOAT, NULL);
	}

	// Setup tex coords to be used for fetching HDR kernel data
	glUseProgram(blurProg);
	GenTexCoordOffsets(screenWidth, screenHeight);
	glUniform2fv(glGetUniformLocation(blurProg, "tc_offset"), 25, texCoordOffsets);

	GenerateOrtho2DMat(nWidth, nHeight);
	glUseProgram(0);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
// Create a matrix that maps geometry to the screen. 1 unit in the x directionequals one pixel 
// of width, same with the y direction.
//
void HDRBloom::GenerateOrtho2DMat(GLuint imageWidth, GLuint imageHeight)
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

///////////////////////////////////////////////////////////////////////////////
// Update the camera based on user input, toggle display modes
// 
void HDRBloom::UpdateMode(void)
{ 
	static CStopWatch timer;
	float fTime = timer.GetElapsedSeconds();
	float linear = fTime / 100;
	float smallLinear = fTime / 1000;

	// Increase the scene exposure
	if(GetAsyncKeyState(VK_UP))
	{
		if((exposure + linear) < 20.0f)
			exposure += linear;
	}
	// Decrease the scene exposure
	if(GetAsyncKeyState(VK_DOWN))
	{
		if((exposure - linear) > 0.01f)
			exposure -= linear;
	}
	
	// Decrease amount of bloom effect
	if(GetAsyncKeyState(VK_LEFT))
	{
		if((bloomLevel - smallLinear) > 0.00f)
			bloomLevel -= smallLinear;
		
	}
	// Increase amount of bloom effect
	if(GetAsyncKeyState(VK_RIGHT))
	{
		if((bloomLevel + smallLinear) < 1.5f)
			bloomLevel += smallLinear;
	}
}



///////////////////////////////////////////////////////////////////////////////
// Render a frame. The owning framework is responsible for buffer swaps,
// flushes, etc.
void HDRBloom::Render(void)
{
	int i =0;
	UpdateMode();

	// first render the scene in HDR to fbo
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdrFBO[0]);
	glDrawBuffers(1, &fboBuffs[0]);
	glClearColor(vSkyBlue[0], vSkyBlue[1], vSkyBlue[2], vSkyBlue[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glDrawBuffers(1, &fboBuffs[1]);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw to two textures, the first contains scene data
	// the second contains only the bright areas
	glDrawBuffers(2, fboBuffs);
	modelViewMatrix.PushMatrix();	
		M3DMatrix44f mCamera;
		cameraFrame.GetCameraMatrix(mCamera);
		modelViewMatrix.MultMatrix(mCamera);
		glBindTexture(GL_TEXTURE_2D, textures[0]); // Marble

		// Draw the floor
		SetupTexReplaceProg(vLightPos, vWhite);
		floorBatch.Draw();

		// Draw the window
		modelViewMatrix.PushMatrix();
			modelViewMatrix.Translate(0.0f, -0.4f, -4.0f);
			modelViewMatrix.Rotate(10.0, 0.0, 1.0, 0.0);
			glBindTexture(GL_TEXTURE_2D, windowTexture); // Window Tex
			
			// First draw the window contents from texture
			SetupTexReplaceProg(vLightPos, vWhiteX2);
			windowBatch.Draw();

			// Now draw the border and the grid
			SetupFlatColorProg(vLightPos, vGrey);
			windowGridBatch.Draw();
			windowBorderBatch.Draw();
		modelViewMatrix.PopMatrix();
	modelViewMatrix.PopMatrix();

	projectionMatrix.PushMatrix();
		projectionMatrix.LoadMatrix(orthoMatrix);
		modelViewMatrix.PushMatrix();
			modelViewMatrix.LoadIdentity();
			
			// Take the data from the brightness texture and 
			// blur it in 4 consecutive passes to textures of 
			// decreasing size
			SetupBlurProg();
			for (i =0; i<4; i++)
			{
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, brightPassFBO[i]); // draws into brightBlurTextures[i+1]
				glDrawBuffers(1, &fboBuffs[0]);
				glViewport(0, 0, screenWidth/((i+1)*3.0), screenHeight/((i+1)*3.0));
				glBindTexture(GL_TEXTURE_2D, brightBlurTextures[i]);
				screenQuad.Draw();
			}

			// Combine original scene with blurred bright textures 
			// to create the bloom effect
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glDrawBuffers(1,windowBuff);
			glViewport(0, 0, screenWidth, screenHeight);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			SetupHDRProg();
			screenQuad.Draw();
		modelViewMatrix.PopMatrix();
	projectionMatrix.PopMatrix();
	
	// Put the texture units back the way they were
	for (i=5; i>-1; i--)
	{
		glActiveTexture(GL_TEXTURE0+i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}
