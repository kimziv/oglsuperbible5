// Triangle.cpp
// Our first OpenGL program that will just draw a triangle on the screen.

#include <GLTools.h>	// OpenGL toolkit

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif


///////////////////////////////////////////////////////////////////////////////
// This function does any needed initialization on the rendering context. 
// This is the first opportunity to do any OpenGL related tasks.
void SetupRC()
	{

	// Black background
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f );
    }





///////////////////////////////////////////////////////////////////////////////
// Called to draw scene
void RenderScene(void)
	{
	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Flush drawing commands
	glutSwapBuffers();
	}



///////////////////////////////////////////////////////////////////////////////
// Window has changed size, or has just been created. In either case, we need
// to use the window dimensions to set the viewport and the projection matrix.
void ChangeSize(int w, int h)
	{
	glViewport(0, 0, w, h);
	}

///////////////////////////////////////////////////////////////////////////////
// Main entry point for GLUT based programs
int main(int argc, char* argv[])
	{
#ifdef __APPLE__
	static char szParentDirectory[255];   	
	
	///////////////////////////////////////////////////////////////////////////   
	// Get the directory where the .exe resides
	char *c;
	strncpy( szParentDirectory, argv[0], sizeof(szParentDirectory) );
	c = (char*) szParentDirectory;
	
	while (*c != '\0')     // go to end 
		c++;
	
	while (*c != '/')      // back up to parent 
		c--;
	
	*c++ = '\0';             // cut off last part (binary name) 
		
	///////////////////////////////////////////////////////////////////////////   
	// Change to Resources directory. Any data files need to be placed there 
	chdir(szParentDirectory);
	chdir("../Resources");
#endif
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(800, 600);
	glutCreateWindow("3D Effects Demo");
	
	GLenum err = glewInit();
	if (GLEW_OK != err)
		{
		// Problem: glewInit failed, something is seriously wrong.
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		return 1;
		}
	
	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);

	SetupRC();

	glutMainLoop();
	return 0;
	}
