// normalsProgram.cpp
// OpenGL SuperBible
// Demonstrates a simple toon shader
// Program by Richard S. Wright Jr.

#include <GLTools.h>    // OpenGL toolkit
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

#ifdef _MSC_VER
#pragma comment (lib, "GLTools.lib")
#endif /* _MSC_VER */

static GLFrame             viewFrame;
static GLFrustum           viewFrustum;
static GLTriangleBatch     torusBatch;
static GLMatrixStack       modelViewMatrix;
static GLMatrixStack       projectionMatrix;
static GLGeometryTransform transformPipeline;
static GLShaderManager     shaderManager;

static GLuint    normalsProgram;                // The dissolving light shader
static GLint    locMVP;                         // The location of the ModelViewProjection matrix uniform
static GLint    locMV;                          // The location of the ModelView matrix uniform
static GLint    locNM;                          // The location of the Normal matrix uniform
static GLint   locColorTable;                   // The location of the color table

static GLuint  flatProgram;
static GLint    locMVP2;                        // The location of the ModelViewProjection matrix uniform
static GLint    locMV2;                         // The location of the ModelView matrix uniform
static GLint    locNM2;                         // The location of the Normal matrix uniform

// This function does any needed initialization on the rendering context.
void SetupRC(void)
{
    // Background
    glClearColor(0.2f, 0.2f, 0.3f, 1.0f );

    glEnable(GL_DEPTH_TEST);

    shaderManager.InitializeStockShaders();
    viewFrame.MoveForward(4.0f);

    // Make the torus
    gltMakeTorus(torusBatch, .80f, 0.25f, 14, 12);

    normalsProgram = gltLoadShaderTripletWithAttributes("../../Data/Shaders/Chapter11/GSNormals.vs",
                                                        "../../Data/Shaders/Chapter11/GSNormals.gs",
                                                        "../../Data/Shaders/Chapter11/GSNormals.fs",
                                                        2,
                                                        GLT_ATTRIBUTE_VERTEX, "vVertex",
                                                        GLT_ATTRIBUTE_NORMAL, "vNormal");

    locMVP = glGetUniformLocation(normalsProgram, "mvpMatrix");
    locMV  = glGetUniformLocation(normalsProgram, "mvMatrix");
    locNM  = glGetUniformLocation(normalsProgram, "normalMatrix");

    flatProgram = gltLoadShaderTripletWithAttributes("../../Data/Shaders/Chapter11/FlatShader.vs",
                                                     NULL,
                                                     "../../Data/Shaders/Chapter11/FlatShader.fs",
                                                     2,
                                                     GLT_ATTRIBUTE_VERTEX, "vVertex",
                                                     GLT_ATTRIBUTE_NORMAL, "vNormal");

    locMVP2 = glGetUniformLocation(flatProgram, "mvpMatrix");
    locMV2  = glGetUniformLocation(flatProgram, "mvMatrix");
    locNM2  = glGetUniformLocation(flatProgram, "normalMatrix");
}

// Cleanup
void ShutdownRC(void)
{
    glDeleteProgram(flatProgram);
    glDeleteProgram(normalsProgram);
}

// Called to draw scene
void RenderScene(void)
{
    static CStopWatch rotTimer;

    // Clear the window and the depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    modelViewMatrix.PushMatrix(viewFrame);
    modelViewMatrix.Rotate(rotTimer.GetElapsedSeconds() * 10.0f, 0.0f, 1.0f, 0.0f);
    modelViewMatrix.Rotate(rotTimer.GetElapsedSeconds() * 13.0f, 1.0f, 0.0f, 0.0f);

    GLfloat vEyeLight[] = { -100.0f, 100.0f, 100.0f };
    GLfloat vAmbientColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    GLfloat vDiffuseColor[] = { 0.1f, 1.0f, 0.1f, 1.0f };
    GLfloat vSpecularColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glUseProgram(flatProgram);
    glUniformMatrix4fv(locMVP2, 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
    glUniformMatrix4fv(locMV2, 1, GL_FALSE, transformPipeline.GetModelViewMatrix());
    glUniformMatrix3fv(locNM2, 1, GL_FALSE, transformPipeline.GetNormalMatrix());

    torusBatch.Draw();

    glUseProgram(normalsProgram);
    glUniformMatrix4fv(locMVP, 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
    glUniformMatrix4fv(locMV, 1, GL_FALSE, transformPipeline.GetModelViewMatrix());
    glUniformMatrix3fv(locNM, 1, GL_FALSE, transformPipeline.GetNormalMatrix());

    torusBatch.Draw();

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
    glutCreateWindow("Geometry Shader Normal Visualizer");
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
