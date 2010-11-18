#include <gltools.h>
#include <Stopwatch.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#include <GL/freeglut_ext.h>
#endif

#ifdef _MSC_VER
#pragma comment (lib, "GLTools.lib")
#endif /* _MSC_VER */

static GLuint               instancingShader;        // The instancing renderer
static GLuint               square_vao;
static GLuint               square_vbo;
static bool                 done = false;
static GLint                angle_loc = 0;

void SetupRC()
{
    instancingShader = gltLoadShaderPair("../../Data/Shaders/Chapter12/instancing.vs",
                                         "../../Data/Shaders/Chapter12/instancing.fs");
    glLinkProgram(instancingShader);
    glUseProgram(instancingShader);
    angle_loc = glGetUniformLocation(instancingShader, "angle");

    static const GLfloat square_vertices[] =
    {
        -1.0f, -1.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 1.0f
    };

    static const GLfloat instance_colors[] =
    {
        1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f
    };

    static const GLfloat instance_positions[] =
    {
        -2.0f, -2.0f, 0.0f, 0.0f,
         2.0f, -2.0f, 0.0f, 0.0f,
         2.0f,  2.0f, 0.0f, 0.0f,
        -2.0f,  2.0f, 0.0f, 0.0f
    };

    GLuint offset = 0;

    glGenVertexArrays(1, &square_vao);
    glGenBuffers(1, &square_vbo);
    glBindVertexArray(square_vao);
    glBindBuffer(GL_ARRAY_BUFFER, square_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square_vertices) + sizeof(instance_colors) + sizeof(instance_positions), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(square_vertices), square_vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(square_vertices), sizeof(instance_colors), instance_colors);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(square_vertices) + sizeof(instance_colors), sizeof(instance_positions), instance_positions);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *)sizeof(square_vertices));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(sizeof(square_vertices) + sizeof(instance_colors)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
}

void RenderScene(void)
{
    static CStopWatch stopwatch;

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(instancingShader);
    glBindVertexArray(square_vao);
    glUniform1f(angle_loc, stopwatch.GetElapsedSeconds());
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, 4);

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
        case 27:
            done = true;
            break;
        default:
            break;
    }
}

// Cleanup
void ShutdownRC(void)
{
    glDeleteBuffers(1, &square_vbo);
    glDeleteVertexArrays(1, &square_vao);
}

///////////////////////////////////////////////////////////////////////////////
// Main entry point for GLUT based programs
int main(int argc, char* argv[])
{
    gltSetWorkingDirectory(argv[0]);

    glutInit(&argc, argv);
    // We're using compatibility profile here because GLUT generates GL errors on
    // a core context. This application DOES NOT require a compatibility context.
    glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
    glutInitContextVersion(3, 3);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Instancing");
    glutReshapeFunc(ChangeSize);
    glutKeyboardFunc(Keyboard);
    glutDisplayFunc(RenderScene);

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }

    SetupRC();

    glutMainLoop();
    ShutdownRC();

    return 0;
}
