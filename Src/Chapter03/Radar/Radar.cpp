// Radar.cpp
// OpenGL SuperBible, Chapter 2
// Demonstrates antialiased lines
// Program by Richard S. Wright Jr.

#include <gltools.h>	// OpenGL toolkit
#include <GLMatrixStack.h>
#include <GLFrame.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLGeometryTransform.h>

#include <math.h>
#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <gl/glut.h>
#endif

/////////////////////////////////////////////////////////////////////////////////
// An assortment of needed classes
GLShaderManager		shaderManager;
GLFrustum			viewFrustum;
GLBatch				batchCenterPoint;
GLBatch				batchLineSweep;
GLBatch				batchRing0;
GLBatch				batchRing1;
GLBatch				batchRing2;
GLBatch				batchRing3;



///////////////////////////////////////////////////////////////////////////////
// This function does any needed initialization on the rendering context. 
// This is the first opportunity to do any OpenGL related tasks.
void SetupRC()
	{
	shaderManager.InitializeStockShaders();

	// Black background
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f );
	glLineWidth(5.5f);
	glPointSize(15.0f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	
	// Center is a big point
	batchCenterPoint.Begin(GL_POINTS, 1);
		batchCenterPoint.Vertex3f(0.0f, 0.0f, 0.0f);
	batchCenterPoint.End();
	
	// Draw concentric rings
	batchRing0.Begin(GL_LINE_LOOP, 100);
	for(GLfloat angle = 0.0f; angle < M3D_2PI; angle += M3D_2PI / 100.0f)
		batchRing0.Vertex3f(cos(angle) * 3.0f, sin(angle) * 3.0f, 0.0f);
	batchRing0.End();
		
	batchRing1.Begin(GL_LINE_LOOP, 100);
		for(GLfloat angle = 0.0f; angle < M3D_2PI; angle += M3D_2PI / 100.0f)
			batchRing1.Vertex3f(cos(angle) * 5.0f, sin(angle) * 5.0f, 0.0f);
	batchRing1.End();

	batchRing2.Begin(GL_LINE_LOOP, 100);
	for(GLfloat angle = 0.0f; angle < M3D_2PI; angle += M3D_2PI / 100.0f)
		batchRing2.Vertex3f(cos(angle) * 7.0f, sin(angle) * 7.0f, 0.0f);
	batchRing2.End();
	
	batchRing3.Begin(GL_LINE_LOOP, 100);
	for(GLfloat angle = 0.0f; angle < M3D_2PI; angle += M3D_2PI / 100.0f)
		batchRing3.Vertex3f(cos(angle) * 9.0f, sin(angle) * 9.0f, 0.0f);
	batchRing3.End();
		
		
	batchLineSweep.Begin(GL_TRIANGLE_FAN, 9*3);
		float alpha = 1.0f;
		float stepSize = M3D_2PI / 5.0f / 9.0f;
		for(GLfloat angle = 0.0f; angle < M3D_2PI / 5.0f; angle += stepSize) {
			// Origin
			batchLineSweep.Color4f(0.0f, 1.0f, 0.0f, 1.0f-alpha);
			batchLineSweep.Vertex3f(0.0f, 0.0f, 0.0f);

			batchLineSweep.Color4f(0.0f, 1.0f, 0.0f, 1.0f - alpha);
			batchLineSweep.Vertex3f(cos(angle) * 9.0f, sin(angle) * 9.0f, 0.0f);
			alpha -= 1.0f / 9.0f;

			batchLineSweep.Color4f(0.0f, 1.0f, 0.0f, 1.0f - alpha);
			batchLineSweep.Vertex3f(cos(angle + stepSize) * 9.0f, sin(angle + stepSize) * 9.0f, 0.0f);
			}
	batchLineSweep.End();
    }




///////////////////////////////////////////////////////////////////////////////
// Called to draw scene
void RenderScene(void)
	{
	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT);

	GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
	
	shaderManager.UseStockShader(GLT_SHADER_FLAT, viewFrustum.GetProjectionMatrix(), vGreen);
	batchCenterPoint.Draw();
	batchRing0.Draw();
	batchRing1.Draw();
	batchRing2.Draw();
	batchRing3.Draw();
	
	shaderManager.UseStockShader(GLT_SHADER_SHADED, viewFrustum.GetProjectionMatrix(), vGreen);
	batchLineSweep.Draw();
	
	static float fOffset = 0.0f;
	fOffset += 0.01f;
batchLineSweep.Reset();
	batchLineSweep.Begin(GL_TRIANGLE_FAN, 9*3);
	float alpha = 1.0f;
	float stepSize = M3D_2PI / 5.0f / 9.0f;
	for(GLfloat angle = 0.0f; angle < M3D_2PI / 5.0f; angle += stepSize) {
		// Origin
		batchLineSweep.Color4f(0.0f, 0.75f, 0.0f, 1.0f-alpha);
		batchLineSweep.Vertex3f(0.0f, 0.0f, 0.0f);
		
		batchLineSweep.Color4f(0.0f, 0.75f, 0.0f, 1.0f - alpha);
		batchLineSweep.Vertex3f(cos(angle + fOffset) * 9.0f, sin(angle+fOffset) * 9.0f, 0.0f);
		alpha -= 1.0f / 9.0f;
		
		batchLineSweep.Color4f(0.0f, 0.75f, 0.0f, 1.0f - alpha);
		batchLineSweep.Vertex3f(cos(angle + stepSize + fOffset) * 9.0f, sin(angle + stepSize + fOffset) * 9.0f, 0.0f);
	}
	batchLineSweep.End();

/*	batchLineSweep.Color4f(0.0f, 1.0f, 0.0f, 1.0f);
	batchLineSweep.Vertex3f(0.0f, 0.0f, 0.0f);
	float alpha = 1.0f;
	for(GLfloat angle = 0.0f; angle < M3D_2PI / 5.0f; angle += M3D_2PI / 5.0f / 9.0f) {
		batchLineSweep.Color4f(0.0f, 1.0f, 0.0f, 1.0f - alpha);
		alpha -= 1.0f / 9.0f;
		batchLineSweep.Vertex3f(cos(angle + fOffset) * 9.0f, sin(angle + fOffset) * 9.0f, 0.0f);
	}
	batchLineSweep.End();
*/
	// Flush drawing commands
	glutSwapBuffers();
	}


///////////////////////////////////////////////////////////////////////////////
// Window has changed size, or has just been created. In either case, we need
// to use the window dimensions to set the viewport and the projection matrix.
void ChangeSize(int w, int h)
	{
	glViewport(0, 0, w, h);
	viewFrustum.SetPerspective(35.0f, float(w) / float(h), 1.0f, 500.0f);
	
	float fAspect = float(w)/float(h);
	float x = 10.0f * fAspect;
	
	viewFrustum.SetOrthographic(x, -x, -10.0f, 10.0f, -1.0f, 1.0f);
	}


void TimerFunction(int value)
	{
    glutPostRedisplay();
    glutTimerFunc(33,TimerFunction, 1);
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
	glutCreateWindow("Aliased and Antialiased Lines");
	
	GLenum err = glewInit();
	if (GLEW_OK != err)
		{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		return 1;
		}
	
	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);
	glutTimerFunc(33, TimerFunction, 1);


	SetupRC();

	glutMainLoop();
	return 0;
	}
