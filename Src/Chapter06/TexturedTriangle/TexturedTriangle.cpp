// TexturedTriangle.cpp
// Our first OpenGL program that will just draw a triangle on the screen.

#include <GLTools.h>            // OpenGL toolkit
#include <GLShaderManager.h>    // Shader Manager Class

#ifdef __APPLE__
#include <glut/glut.h>          // OS X version of GLUT
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>            // Windows FreeGlut equivalent
#endif

GLBatch	triangleBatch;
GLShaderManager	shaderManager;

GLint	myTexturedIdentityShader;
GLuint	textureID;

///////////////////////////////////////////////////////////////////////////////
// Window has changed size, or has just been created. In either case, we need
// to use the window dimensions to set the viewport and the projection matrix.
void ChangeSize(int w, int h)
    {
	glViewport(0, 0, w, h);
    }


// Load a TGA as a 2D Texture. Completely initialize the state
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
    
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, nComponents, nWidth, nHeight, 0,
				 eFormat, GL_UNSIGNED_BYTE, pBits);
	
    free(pBits);
    
    if(minFilter == GL_LINEAR_MIPMAP_LINEAR || 
       minFilter == GL_LINEAR_MIPMAP_NEAREST ||
       minFilter == GL_NEAREST_MIPMAP_LINEAR ||
       minFilter == GL_NEAREST_MIPMAP_NEAREST)
        glGenerateMipmap(GL_TEXTURE_2D);
    
	return true;
}


///////////////////////////////////////////////////////////////////////////////
// This function does any needed initialization on the rendering context. 
// This is the first opportunity to do any OpenGL related tasks.
void SetupRC()
	{
	// Blue background
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f );
    
	shaderManager.InitializeStockShaders();

	// Load up a triangle
	GLfloat vVerts[] = { -0.5f, 0.0f, 0.0f, 
		                  0.5f, 0.0f, 0.0f,
						  0.0f, 0.5f, 0.0f };

	GLfloat vTexCoords [] = { 0.0f, 0.0f,
		                      1.0f, 0.0f, 
						      0.5f, 1.0f };

	triangleBatch.Begin(GL_TRIANGLES, 3, 1);
	triangleBatch.CopyVertexData3f(vVerts);
	triangleBatch.CopyTexCoordData2f(vTexCoords, 0);
	triangleBatch.End();

	myTexturedIdentityShader = gltLoadShaderPairWithAttributes("TexturedIdentity.vp", "TexturedIdentity.fp", 2, 
		                            GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_TEXTURE0, "vTexCoords");

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	LoadTGATexture("stone.tga", GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
	}


///////////////////////////////////////////////////////////////////////////////
// Cleanup
void ShutdownRC()
   {
   glDeleteProgram(myTexturedIdentityShader);
   glDeleteTextures(1, &textureID);
   }


///////////////////////////////////////////////////////////////////////////////
// Called to draw scene
void RenderScene(void)
	{
	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glUseProgram(myTexturedIdentityShader);
    glBindTexture(GL_TEXTURE_2D, textureID);
    GLint iTextureUniform = glGetUniformLocation(myTexturedIdentityShader, "colorMap");
	glUniform1i(iTextureUniform, 0);

	triangleBatch.Draw();

	// Perform the buffer swap to display back buffer
	glutSwapBuffers();
	}


///////////////////////////////////////////////////////////////////////////////
// Main entry point for GLUT based programs
int main(int argc, char* argv[])
	{
	gltSetWorkingDirectory(argv[0]);
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Textured Triangle");
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
		return 1;
		}
	
	SetupRC();

	glutMainLoop();

	ShutdownRC();

	return 0;
	}
