#include "instancing.h"

InstancingApp::InstancingApp(void)
{

}

void InstancingApp::Initialize(void)
{
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }

    instancingProg = gltLoadShaderPair("instancing.vs", "instancing.fs");
    glBindAttribLocation(instancingProg, 0, "position");
    glBindAttribLocation(instancingProg, 1, "instance_color");
    glBindAttribLocation(instancingProg, 2, "instance_position");
    glLinkProgram(instancingProg);
    glUseProgram(instancingProg);

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
    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(square_vertices), square_vertices);
    offset += sizeof(square_vertices);
    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(instance_colors), instance_colors);
    offset += sizeof(instance_colors);
    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(instance_positions), instance_positions);
    offset += sizeof(instance_positions);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *)sizeof(square_vertices));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(sizeof(square_vertices) + sizeof(instance_colors)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glVertexAttribDivisorARB(1, 1);
    glVertexAttribDivisorARB(2, 1);
}

void InstancingApp::Shutdown(void)
{
    glDeleteBuffers(1, &square_vbo);
    glDeleteVertexArrays(1, &square_vao);
}

void InstancingApp::Resize(GLsizei nWidth, GLsizei nHeight)
{
    screenWidth = nWidth;
    screenHeight = nHeight;
    glViewport(0, 0, nWidth, nHeight);
}

void InstancingApp::Render(void)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(instancingProg);
    glBindVertexArray(square_vao);
    glDrawArraysInstancedARB(GL_TRIANGLE_FAN, 0, 4, 4);
}
