// ToonShader.cpp
// OpenGL SuperBible
// Demonstrates a simple toon shader
// Program by Richard S. Wright Jr.

#include <GLTools.h>	// OpenGL toolkit
#include <GLMatrixStack.h>
#include <GLFrame.h>
#include <GLFrustum.h>
#include <GLGeometryTransform.h>
#include <Stopwatch.h>

#include <math.h>
#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif


GLFrame             viewFrame;
GLFrustum           viewFrustum;
GLTriangleBatch     torusBatch;
GLBatch             cubeBatch;
GLMatrixStack       modelViewMatrix;
GLMatrixStack       projectionMatrix;
GLGeometryTransform transformPipeline;
GLShaderManager     shaderManager;

GLuint	toonShader;	        // The dissolving light shader
GLint	locLight;			// The location of the Light in eye coordinates
GLint	locMVP;				// The location of the ModelViewProjection matrix uniform
GLint	locMV;				// The location of the ModelView matrix uniform
GLint	locNM;				// The location of the Normal matrix uniform
GLint   locColorTable;		// The location of the color table

GLuint  flatProgram;
GLint	locMVP2;				// The location of the ModelViewProjection matrix uniform
GLint	locMV2;				// The location of the ModelView matrix uniform
GLint	locNM2;				// The location of the Normal matrix uniform

GLuint	texture;

GLuint vao;
GLuint vertex_buffer;
GLuint element_buffer;

// This function does any needed initialization on the rendering
// context. 
void SetupRC(void)
	{
	// Background
	glClearColor(0.2f, 0.2f, 0.3f, 1.0f );

	glEnable(GL_DEPTH_TEST);

    shaderManager.InitializeStockShaders();
    viewFrame.MoveForward(4.0f);

    // Make the torus
    gltMakeTorus(torusBatch, .70f, 0.10f, 11, 7);
    gltMakeCube(cubeBatch, 1.0f);

    toonShader = gltLoadShaderTripletWithAttributes("GSTessellate.vs", "GSTessellate.gs", "GSTessellate.fs", 2, GLT_ATTRIBUTE_VERTEX, "vVertex",
    GLT_ATTRIBUTE_NORMAL, "vNormal");

    locMVP = glGetUniformLocation(toonShader, "mvpMatrix");
    locMV  = glGetUniformLocation(toonShader, "mvMatrix");
    locNM  = glGetUniformLocation(toonShader, "normalMatrix");

    static const GLfloat vertices[] =
    {
        -1.0f, -1.0f, -1.0f,        // A
        -1.0f, -1.0f,  1.0f,        // B
        -1.0f,  1.0f, -1.0f,        // C
        -1.0f,  1.0f,  1.0f,        // D
         1.0f, -1.0f, -1.0f,        // E
         1.0f, -1.0f,  1.0f,        // F
         1.0f,  1.0f, -1.0f,        // G
         1.0f,  1.0f,  1.0f         // H
    };

    static const GLshort indices[] =
    {
        0, 1, 2,
        3, 2, 1,
        1, 5, 3,
        7, 3, 5,
        5, 4, 7,
        6, 7, 4,
        4, 0, 6,
        2, 6, 0,
        4, 5, 0,
        1, 0, 5,
        3, 7, 2,
        6, 2, 7
    };

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    glGenBuffers(1, &element_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    GLenum e = glGetError();
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
        GLenum e;

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    modelViewMatrix.PushMatrix(viewFrame);
		modelViewMatrix.Rotate(rotTimer.GetElapsedSeconds() * 20.0f, 0.0f, 1.0f, 0.0f);
		modelViewMatrix.Rotate(rotTimer.GetElapsedSeconds() * 17.0f, 1.0f, 0.0f, 0.0f);

		GLfloat vEyeLight[] = { -100.0f, 100.0f, 100.0f };
		GLfloat vAmbientColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
		GLfloat vDiffuseColor[] = { 0.1f, 1.0f, 0.1f, 1.0f };
		GLfloat vSpecularColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

        glUseProgram(toonShader);
		// glUniform3fv(locLight, 1, vEyeLight);
		glUniformMatrix4fv(locMVP, 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
		glUniformMatrix4fv(locMV, 1, GL_FALSE, transformPipeline.GetModelViewMatrix());
		glUniformMatrix3fv(locNM, 1, GL_FALSE, transformPipeline.GetNormalMatrix());

        e = glGetError();

    // torusBatch.Draw();
    // cubeBatch.Draw();
        glDisable(GL_CULL_FACE);
        e = glGetError();
        glBindVertexArray(vao);
        e = glGetError();
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, NULL);
        e = glGetError();

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
	glutCreateWindow("Geometry Shader Tessellation");
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
