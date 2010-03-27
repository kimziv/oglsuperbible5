// ProvokingVertex.cpp
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

GLint	myIdentityShader;

///////////////////////////////////////////////////////////////////////////////
// Window has changed size, or has just been created. In either case, we need
// to use the window dimensions to set the viewport and the projection matrix.
void ChangeSize(int w, int h)
    {
	glViewport(0, 0, w, h);
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

	GLfloat vColors [] = { 1.0f, 0.0f, 0.0f, 1.0f,
		                   0.0f, 1.0f, 0.0f, 1.0f,
						   0.0f, 0.0f, 1.0f, 1.0f };

	triangleBatch.Begin(GL_TRIANGLES, 3);
	triangleBatch.CopyVertexData3f(vVerts);
	triangleBatch.CopyColorData4f(vColors);
	triangleBatch.End();

	myIdentityShader = gltLoadShaderPairWithAttributes("ProvokingVertex.vp", "ProvokingVertex.fp", 2, 
		                            GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_COLOR, "vColor");

	glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
	}


///////////////////////////////////////////////////////////////////////////////
// Cleanup
void ShutdownRC()
   {
   glDeleteProgram(myIdentityShader);

   }


///////////////////////////////////////////////////////////////////////////////
// Called to draw scene
void RenderScene(void)
	{
	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glUseProgram(myIdentityShader);
	triangleBatch.Draw();

	// Perform the buffer swap to display back buffer
	glutSwapBuffers();
	}

int nToggle = 1;
void KeyPressFunc(unsigned char key, int x, int y)
	{
	if(key == 32)
		{
		nToggle++;

		if(nToggle %2 == 0) {
			glProvokingVertex(GL_LAST_VERTEX_CONVENTION);
			glutSetWindowTitle("Provoking Vertex - Last Vertex - Press Space Bars");
			}
		else {
			glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
			glutSetWindowTitle("Provoking Vertex - First Vertex - Press Space Bars");
			}

		glutPostRedisplay();
		}
	}
       


///////////////////////////////////////////////////////////////////////////////
// Main entry point for GLUT based programs
int main(int argc, char* argv[])
	{
	gltSetWorkingDirectory(argv[0]);
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Provoking Vertex - First Vertex - Press Space Bars");
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
	glutKeyboardFunc(KeyPressFunc);


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
