// LitTexture.cpp
// OpenGL SuperBible
// Demonstrates combining lighting and texture
// Program by Richard S. Wright Jr.

#include <GLTools.h>	// OpenGL toolkit
#include <GLMatrixStack.h>
#include <GLFrame.h>
#include <GLFrustum.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>

#include <math.h>
#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif


GLFrame             viewFrame;
GLFrustum           viewFrustum;
GLTriangleBatch     sphereBatch;
GLMatrixStack       modelViewMatrix;
GLMatrixStack       projectionMatrix;
GLGeometryTransform transformPipeline;
GLShaderManager     shaderManager;

GLuint	ADSTextureShader;	// The textured diffuse light shader
GLint	locAmbient;			// The location of the ambient color
GLint   locDiffuse;			// The location of the diffuse color
GLint   locSpecular;		// The location of the specular color
GLint	locLight;			// The location of the Light in eye coordinates
GLint	locMVP;				// The location of the ModelViewProjection matrix uniform
GLint	locMV;				// The location of the ModelView matrix uniform
GLint	locNM;				// The location of the Normal matrix uniform
GLint   locTexture;
GLuint  texture;

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

// This function does any needed initialization on the rendering
// context. 
void SetupRC(void)
	{
	// Background
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f );

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

    shaderManager.InitializeStockShaders();
    viewFrame.MoveForward(4.0f);

    // Make the sphere
    gltMakeSphere(sphereBatch, 1.0f, 26, 13);

	ADSTextureShader = gltLoadShaderPairWithAttributes("ADSTexture.vp", "ADSTexture.fp", 3, GLT_ATTRIBUTE_VERTEX, "vVertex",
			GLT_ATTRIBUTE_NORMAL, "vNormal", GLT_ATTRIBUTE_TEXTURE0, "vTexture0");

	locAmbient = glGetUniformLocation(ADSTextureShader, "ambientColor");
	locDiffuse = glGetUniformLocation(ADSTextureShader, "diffuseColor");
	locSpecular = glGetUniformLocation(ADSTextureShader, "specularColor");
	locLight = glGetUniformLocation(ADSTextureShader, "vLightPosition");
	locMVP = glGetUniformLocation(ADSTextureShader, "mvpMatrix");
	locMV  = glGetUniformLocation(ADSTextureShader, "mvMatrix");
	locNM  = glGetUniformLocation(ADSTextureShader, "normalMatrix");
	locTexture = glGetUniformLocation(ADSTextureShader, "colorMap");

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	LoadTGATexture("CoolTexture.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
	}

// Cleanup
void ShutdownRC(void)
   {
   glDeleteTextures(1, &texture);
   }


// Called to draw scene
void RenderScene(void)
	{
	static CStopWatch rotTimer;

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
    modelViewMatrix.PushMatrix(viewFrame);
		modelViewMatrix.Rotate(rotTimer.GetElapsedSeconds() * 10.0f, 0.0f, 1.0f, 0.0f);

		GLfloat vEyeLight[] = { -100.0f, 100.0f, 100.0f };
		GLfloat vAmbientColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
		GLfloat vDiffuseColor[] = { 1.0f, 1.0f, 1.0f, 1.0f};
		GLfloat vSpecularColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

		glBindTexture(GL_TEXTURE_2D, texture);
		glUseProgram(ADSTextureShader);
		glUniform4fv(locAmbient, 1, vAmbientColor);
		glUniform4fv(locDiffuse, 1, vDiffuseColor);
		glUniform4fv(locSpecular, 1, vSpecularColor);
		glUniform3fv(locLight, 1, vEyeLight);
		glUniformMatrix4fv(locMVP, 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
		glUniformMatrix4fv(locMV, 1, GL_FALSE, transformPipeline.GetModelViewMatrix());
		glUniformMatrix3fv(locNM, 1, GL_FALSE, transformPipeline.GetNormalMatrix());
		glUniform1i(locTexture, 0);
    sphereBatch.Draw();

    modelViewMatrix.PopMatrix();

    glutSwapBuffers();
	glutPostRedisplay();
	}



void ChangeSize(int w, int h)
	{
	// Prevent a divide by zero
	if(h == 0)
		h = 1;

	// Set Viewport to window dimensions
    glViewport(0, 0, w, h);

    viewFrustum.SetPerspective(35.0f, float(w)/float(h), 1.0f, 100.0f);
    
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
	}

///////////////////////////////////////////////////////////////////////////////
// Main entry point for GLUT based programs
int main(int argc, char* argv[])
    {
	gltSetWorkingDirectory(argv[0]);
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Lit Texture");
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
