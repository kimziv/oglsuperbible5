// perspectiveShader.cpp
// OpenGL SuperBible
// Demonstrates the effect of the 'noperspective' interpolation qualifier
// Program by Graham Sellers

#include <GLTools.h>    // OpenGL toolkit
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
#include <GL/freeglut_ext.h>
#endif


GLFrame             viewFrame;
GLFrustum           viewFrustum;
GLMatrixStack       modelViewMatrix;
GLMatrixStack       projectionMatrix;
GLGeometryTransform transformPipeline;

GLuint  perspectiveShader;      // The perspective demonstration shader
GLint   locMVP;                 // The location of the ModelViewProjection matrix uniform
GLint   locUsePerspective;      // Use perspective correction

GLuint  texture;                // The checkerboard texture

GLuint  vao;                    // The VAO
GLuint  vertexBuffer;           // Geometry VBO

GLint   usePerspective = 1;     // Perspective correction toggle

// This function does any needed initialization on the rendering
// context. 
void SetupRC(void)
{
    // Background
    glClearColor(0.2f, 0.2f, 0.3f, 1.0f );

    glEnable(GL_DEPTH_TEST);

    viewFrame.MoveForward(5.0f);

    perspectiveShader = gltLoadShaderPairWithAttributes("Perspective.vs", "Perspective.fs",
                                                        2,
                                                        GLT_ATTRIBUTE_VERTEX, "vVertex",
                                                        GLT_ATTRIBUTE_TEXTURE0, "vTexCoord");

    locMVP = glGetUniformLocation(perspectiveShader, "mvpMatrix");
    locUsePerspective = glGetUniformLocation(perspectiveShader, "usePerspective");

    // Geometry for a simple quad
    static const GLfloat quad[] =
    {
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
    };

    static const GLfloat texcoords[] =
    {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };

    // A checkerboard texture
    static const GLubyte texture_data[] =
    {
        0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255,
        255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0,
        0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255,
        255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0,
        0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255,
        255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0,
        0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255,
        255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0,
        0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255,
        255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0,
        0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255,
        255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0,
        0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255,
        255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0,
        0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255,
        255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0
    };

    // Greate a vertex array object and a vertex buffer for the quad
    // including position and texture coordinates
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad) + sizeof(texcoords), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quad), quad);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(quad), sizeof(texcoords), texcoords);
    glVertexAttribPointer(GLT_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glVertexAttribPointer(GLT_ATTRIBUTE_TEXTURE0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)sizeof(quad));
    glEnableVertexAttribArray(GLT_ATTRIBUTE_VERTEX);
    glEnableVertexAttribArray(GLT_ATTRIBUTE_TEXTURE0);

    // Create the checkerboard texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 16, 16, 0, GL_RED, GL_UNSIGNED_BYTE, texture_data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

// Cleanup
void ShutdownRC(void)
{
    glDeleteTextures(1, &texture);
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteVertexArrays(1, &vao);
}

// Called to draw scene
void RenderScene(void)
{
    static CStopWatch rotTimer;

    // Clear the window and the depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    modelViewMatrix.PushMatrix(viewFrame);
    modelViewMatrix.Rotate(rotTimer.GetElapsedSeconds() * 30.0f, 0.0f, 1.0f, 0.0f);

    glUseProgram(perspectiveShader);
    glUniformMatrix4fv(locMVP, 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
    glUniform1i(locUsePerspective, usePerspective);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

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

void Keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
        // Space toggles perspective correction
        case ' ':   usePerspective ^= 1;
            if (usePerspective)
                glutSetWindowTitle("Perspective Correction (Perspective Correct) - SPACE Toggles");
            else
                glutSetWindowTitle("Perspective Correction (noperspective) - SPACE Toggles");
            break;
        default:
            break;
    };
}

///////////////////////////////////////////////////////////////////////////////
// Main entry point for GLUT based programs
int main(int argc, char* argv[])
{
    gltSetWorkingDirectory(argv[0]);

    glutInit(&argc, argv);
    glutInitContextVersion(3, 2);
      // glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Perspective Correction (Perspective Correct) - SPACE Toggles");
    glutReshapeFunc(ChangeSize);
    glutKeyboardFunc(Keyboard);
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
