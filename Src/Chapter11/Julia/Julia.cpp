// juliaShader.cpp
// OpenGL SuperBible
// Demonstrates the effect of the 'noperspective' interpolation qualifier
// Program by Graham Sellers

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
#include <GL/freeglut_ext.h>
#endif

GLFrame             viewFrame;
/*
GLFrustum           viewFrustum;
GLMatrixStack       modelViewMatrix;
GLMatrixStack       projectionMatrix;
GLGeometryTransform transformPipeline;
*/

GLuint  juliaShader;        // The perspective demonstration shader
GLint   locC;               // The location of the Julia constant C uniform
GLint   locUsePerspective;  // The location of the gradient texture uniform
GLint   locZoom;            // Location of the zoom uniform
GLint   locOffset;          // Location of the offset uniform

GLuint  vao;                // The VAO
GLuint  vertexBuffer;       // Geometry VBO

GLuint  gradient_texture;   // The gradient texture

bool paused = false;
float time_offset = 50.0f;
float zoom = 1.0f;
float x_offset = 0.0f;
float y_offset = 0.0f;

// This function does any needed initialization on the rendering
// context. 
void SetupRC(void)
{
    // Background
    glClearColor(0.2f, 0.2f, 0.3f, 1.0f );

    glEnable(GL_DEPTH_TEST);

    viewFrame.MoveForward(5.0f);

    juliaShader = gltLoadShaderPairWithAttributes("Julia.vs", "Julia.fs",
                                                  1,
                                                  GLT_ATTRIBUTE_VERTEX, "vVertex");

    locC = glGetUniformLocation(juliaShader, "C");
    locZoom = glGetUniformLocation(juliaShader, "zoom");
    locOffset = glGetUniformLocation(juliaShader, "offset");

    // Geometry for a simple quad
    static const GLfloat quad[] =
    {
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
    };

    // Greate a vertex array object and a vertex buffer for the quad
    // including position and texture coordinates
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glVertexAttribPointer(GLT_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(GLT_ATTRIBUTE_VERTEX);

    unsigned char * texture_data = new unsigned char [4096];
    FILE * f = fopen("E:/source/oglsuperbible/VisualStudio2008/Chapter11/Julia/palette.raw", "rb");
    fread(texture_data, 1, 256 * 3, f);
    fclose(f);

    // Setup the gradient texture
    glGenTextures(1, &gradient_texture);
    glBindTexture(GL_TEXTURE_1D, gradient_texture);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_data);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    delete [] texture_data;
}

// Cleanup
void ShutdownRC(void)
{
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteVertexArrays(1, &vao);
}

// Called to draw scene
void RenderScene(void)
{
    static CStopWatch stopwatch;
    static float t = 0.0f;
    float r;

    if (!paused)
        t = stopwatch.GetElapsedSeconds();

    r = t + time_offset;

    float C[2] = { (sinf(r * 0.1f) + cosf(r * 0.23f)) * 0.5f, (cosf(r * 0.13f) + sinf(r * 0.21f)) * 0.5f };
    float offset[2] = { x_offset, y_offset };

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    glUseProgram(juliaShader);
    glUniform2fv(locC, 1, C);
    glUniform2fv(locOffset, 1, offset);
    glUniform1f(locZoom, zoom);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

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
}

void Keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
/*
// Space toggles perspective correction
        case ' ':   usePerspective ^= 1;
            if (usePerspective)
                glutSetWindowTitle("Perspective Correction (Perspective Correct) - SPACE Toggles");
            else
                glutSetWindowTitle("Perspective Correction (noperspective) - SPACE Toggles");
            break;
            */
        case ' ': paused = !paused;
            break;
        case '+': time_offset -= 0.00001f;
            break;
        case '-': time_offset += 0.00001f;
            break;
        case '9': time_offset += 0.0001f;
            break;
        case '3': time_offset -= 0.0001f;
            break;
        case '8': time_offset += 0.01f;
            break;
        case '2': time_offset -= 0.01f;
            break;
        case '7': time_offset += 1.0f;
            break;
        case '1': time_offset -= 1.0f;
            break;
        case ']': zoom *= 1.1f;
            break;
        case '[': zoom /= 1.1f;
            break;
        case 'w': y_offset += zoom;
            break;
        case 'a': x_offset += zoom;
            break;
        case 's': y_offset -= zoom;
            break;
        case 'd': x_offset -= zoom;
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
    glutCreateWindow("Julia Set Renderer");
    glutReshapeFunc(ChangeSize);
    glutKeyboardFunc(Keyboard);
    glutDisplayFunc(RenderScene);

    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW Error: %s/n", glewGetErrorString(err));
        return 1;
    }

    SetupRC();

    glutMainLoop();
    ShutdownRC();

    return 0;
}
