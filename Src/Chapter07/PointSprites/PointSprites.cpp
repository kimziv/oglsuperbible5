// PointSprites.cpp
// OpenGL SuperBible
// Demonstrates Point Sprites via a flythrough star field
// Program by Richard S. Wright Jr.

#include <GLTools.h>	// OpenGL toolkit
#include <GLFrustum.h>
#include <Stopwatch.h>

#include <math.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif


GLFrustum           viewFrustum;
GLBatch             starsBatch;

GLuint	starFieldShader;	// The point sprite shader
GLint	locMVP;				// The location of the ModelViewProjection matrix uniform
GLint   locTimeStamp;       // The location of the time stamp
GLint	locTexture;			// The location of the  texture uniform

GLuint	starTexture;		// The star texture texture object


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

	//glEnable(GL_DEPTH_TEST);
	glEnable(GL_POINT_SPRITE);


	GLfloat fColors[4][4] = {{ 1.0f, 1.0f, 1.0f, 1.0f}, // White
                             { 0.67f, 0.68f, 0.82f, 1.0f}, // Blue Stars
                             { 1.0f, 0.5f, 0.5f, 1.0f}, // Reddish
	                         { 1.0f, 0.82f, 0.65f, 1.0f}}; // Orange


    // Randomly place the stars in their initial positions, and pick a random color
    starsBatch.Begin(GL_POINTS, 10000);
    for(int i = 0; i < 10000; i++)
        {
        // Pick a random color
        switch(rand() %7) {
                case 0:
                case 1:
                case 2:
                starsBatch.Color4fv(fColors[0]);
                break;
                
                case 3:
                case 4:
                starsBatch.Color4fv(fColors[1]);
                break;
                
                case 5:
                starsBatch.Color4fv(fColors[2]);
                break;
                
                case 6:
				default:
                starsBatch.Color4fv(fColors[3]);
                break;
                }
        float x = (800.0f - float(rand() % 1600))*.1f;
        float y = (800.0f - float(rand() % 1600))*.1f;
        float z = -99.0f; //-float(rand() % 500)*.1f;
        starsBatch.Vertex3f(x, y, z);        
        }
    starsBatch.End();


    starFieldShader = gltLoadShaderPairWithAttributes("SpaceFlight.vp", "SpaceFlight.fp", 2, GLT_ATTRIBUTE_VERTEX, "vVertex",
			GLT_ATTRIBUTE_COLOR, "vColor");

	locMVP = glGetUniformLocation(starFieldShader, "mvpMatrix");
	locTexture = glGetUniformLocation(starFieldShader, "starImage");
    locTimeStamp = glGetUniformLocation(starFieldShader, "timeStamp");
    
	glGenTextures(1, &starTexture);
	glBindTexture(GL_TEXTURE_2D, starTexture);
	LoadTGATexture("Star.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    }

// Cleanup
void ShutdownRC(void)
{
    glDeleteTextures(1, &starTexture);
}


// Called to draw scene
void RenderScene(void)
	{
	static CStopWatch timer;

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

    glUseProgram(starFieldShader);
    glUniformMatrix4fv(locMVP, 1, GL_FALSE, viewFrustum.GetProjectionMatrix());
    glUniform1i(locTexture, 0);
    glUniform1f(locTimeStamp, timer.GetElapsedSeconds()*10.0f);

    starsBatch.Draw();
    
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
	}

///////////////////////////////////////////////////////////////////////////////
// Main entry point for GLUT based programs
int main(int argc, char* argv[])
    {
	gltSetWorkingDirectory(argv[0]);
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Spaced Out");
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
