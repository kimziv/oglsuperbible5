// DiffuseLight.cpp
// OpenGL SuperBible
// Demonstrates simple diffuse lighting
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

GLuint	diffuseLightShader;	// The diffuse light shader
GLint	locColor;			// The location of the diffuse color
GLint	locLight;			// The location of the Light in eye coordinates
GLint	locMVP;				// The location of the ModelViewProjection matrix uniform
GLint	locMV;				// The location of the ModelView matrix uniform
GLint	locNM;				// The location of the Normal matrix uniform


// This function does any needed initialization on the rendering
// context. 
void SetupRC(void)
	{
	// Background
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f );

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

    shaderManager.InitializeStockShaders();
    viewFrame.MoveForward(4.0f);

    // Make the sphere
    gltMakeSphere(sphereBatch, 1.0f, 26, 13);

	diffuseLightShader = shaderManager.LoadShaderPairWithAttributes("DiffuseLight.vp", "DiffuseLight.fp", 2, GLT_ATTRIBUTE_VERTEX, "vVertex",
			GLT_ATTRIBUTE_NORMAL, "vNormal");

	locColor = glGetUniformLocation(diffuseLightShader, "diffuseColor");
	locLight = glGetUniformLocation(diffuseLightShader, "vLightPosition");
	locMVP = glGetUniformLocation(diffuseLightShader, "mvpMatrix");
	locMV  = glGetUniformLocation(diffuseLightShader, "mvMatrix");
	locNM  = glGetUniformLocation(diffuseLightShader, "normalMatrix");
	}

// Cleanup
void ShutdownRC(void)
{

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
		GLfloat vDiffuseColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };

		glUseProgram(diffuseLightShader);
		glUniform4fv(locColor, 1, vDiffuseColor);
		glUniform3fv(locLight, 1, vEyeLight);
		glUniformMatrix4fv(locMVP, 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
		glUniformMatrix4fv(locMV, 1, GL_FALSE, transformPipeline.GetModelViewMatrix());
		glUniformMatrix3fv(locNM, 1, GL_FALSE, transformPipeline.GetNormalMatrix());
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
	glutCreateWindow("Simple Diffuse Lighting");
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
