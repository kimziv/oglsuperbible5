#include "explode.h"

ConvolutionApp::ConvolutionApp(void)
    : screenWidth(1024 + 16), screenHeight(768 + 32)
{

}

void ConvolutionApp::Initialize(void)
{
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }

    instancingProg = gltLoadShaderPair("convolve.vs", "convolve.fs");
    glBindAttribLocation(instancingProg, 0, "position");
    glLinkProgram(instancingProg);

    absValueProg = gltLoadShaderPair("convolve.vs", "absvalue.fs");
    glBindAttribLocation(instancingProg, 0, "position");
    glLinkProgram(instancingProg);

    glUseProgram(instancingProg);

    GLbyte *pBits;
    int nWidth, nHeight, nComponents;
    GLenum eFormat;

    // Read the texture bits
    pBits = gltReadTGABits("image.tga", &nWidth, &nHeight, &nComponents, &eFormat);

    glGenTextures(1, &sourceTexture);
    glBindTexture(GL_TEXTURE_2D, sourceTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, nComponents, nWidth, nHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBits);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, &intermediateFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
    glGenTextures(1, &intermediateTexture);
    glBindTexture(GL_TEXTURE_2D, intermediateTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1027, 768, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, intermediateTexture, 0);

    glGenFramebuffers(1, &targetFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);
    glGenTextures(1, &targetTexture);
    glBindTexture(GL_TEXTURE_2D, targetTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1027, 768, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, targetTexture, 0);

    /*
    static const GLfloat kernel[] =
    {
        0.015625, 0.09375, 0.234375, 0.3125, 0.234375, 0.09375, 0.015625
    };
    */

    static const GLfloat gaussian_kernel[] =
    {
        0.004f,
        0.008f,
        0.014f,
        0.022f,
        0.033f,
        0.046f,
        0.061f,
        0.076f,
        0.089f,
        0.098f,
        0.101f,
        0.098f,
        0.089f,
        0.076f,
        0.061f,
        0.046f,
        0.033f,
        0.022f,
        0.014f,
        0.008f,
        0.004f
    };

    /*
    static const GLfloat gaussian_kernel[] =
    {
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f
    };
    */

    // glUniform2f(glGetUniformLocation(instancingProg, "tc_scale"), 1.0f / (float)nWidth, 0.0f); // 1.0f / (float)nHeight);
    // glUniform2f(glGetUniformLocation(instancingProg, "tc_scale"), 0.0f, 1.0f / (float)nHeight);
    kernelScale[0] = 1.0f / (float)nWidth;
    kernelScale[1] = 1.0f / (float)nHeight;
    glUniform1i(glGetUniformLocation(instancingProg, "tex_input_image"), 0);
    glUniform1i(glGetUniformLocation(instancingProg, "tbo_coefficient"), 1);

    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &gaussian_kernelTexture);
    glBindTexture(GL_TEXTURE_BUFFER, gaussian_kernelTexture);
    glGenBuffers(1, &gaussian_kernelBuffer);
    glBindBuffer(GL_TEXTURE_BUFFER, gaussian_kernelBuffer);
    glBufferData(GL_TEXTURE_BUFFER, sizeof(gaussian_kernel), gaussian_kernel, GL_STATIC_DRAW);
    glTexBufferARB(GL_TEXTURE_BUFFER, GL_R32F, gaussian_kernelBuffer);

    static const GLfloat sobel_kernel1[] =
    {
        1.0f, 2.0, 1.0f
    };

    static const GLfloat sobel_kernel2[] =
    {
        1.0f, 0.0f, -1.0f
    };

    glGenTextures(1, &sobel_kernelTexture1);
    glBindTexture(GL_TEXTURE_BUFFER, sobel_kernelTexture1);
    glGenBuffers(1, &sobel_kernelBuffer1);
    glBindBuffer(GL_TEXTURE_BUFFER, sobel_kernelBuffer1);
    glBufferData(GL_TEXTURE_BUFFER, sizeof(sobel_kernel1), sobel_kernel1, GL_STATIC_DRAW);
    glTexBufferARB(GL_TEXTURE_BUFFER, GL_R32F, sobel_kernelBuffer1);

    glGenTextures(1, &sobel_kernelTexture2);
    glBindTexture(GL_TEXTURE_BUFFER, sobel_kernelTexture2);
    glGenBuffers(1, &sobel_kernelBuffer2);
    glBindBuffer(GL_TEXTURE_BUFFER, sobel_kernelBuffer2);
    glBufferData(GL_TEXTURE_BUFFER, sizeof(sobel_kernel2), sobel_kernel2, GL_STATIC_DRAW);
    glTexBufferARB(GL_TEXTURE_BUFFER, GL_R32F, sobel_kernelBuffer2);

    static const GLfloat square_vertices[] =
    {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f
    };

    GLuint offset = 0;

    glGenVertexArrays(1, &square_vao);
    glGenBuffers(1, &square_vbo);
    glBindVertexArray(square_vao);
    glBindBuffer(GL_ARRAY_BUFFER, square_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square_vertices), square_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(0);
}

void ConvolutionApp::Shutdown(void)
{
    glDeleteBuffers(1, &square_vbo);
    glDeleteVertexArrays(1, &square_vao);
}

void ConvolutionApp::Resize(GLsizei nWidth, GLsizei nHeight)
{
    screenWidth = nWidth;
    screenHeight = nHeight;
    glViewport(0, 0, nWidth, nHeight);
}

void ConvolutionApp::Render(void)
{
    glUseProgram(instancingProg);
    glBindVertexArray(square_vao);

    /*
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_BUFFER, gaussian_kernelTexture);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sourceTexture);

    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUniform1i(glGetUniformLocation(instancingProg, "kernel_size"), 21);
    glUniform2f(glGetUniformLocation(instancingProg, "tc_scale"), 0.0f, kernelScale[1]);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, intermediateTexture);

    glUniform2f(glGetUniformLocation(instancingProg, "tc_scale"), kernelScale[0], 0.0f);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    /*/
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_BUFFER, sobel_kernelTexture1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sourceTexture);

    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUniform1i(glGetUniformLocation(instancingProg, "kernel_size"), 3);
    glUniform2f(glGetUniformLocation(instancingProg, "tc_scale"), 0.0f, kernelScale[1]);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);
    glBindTexture(GL_TEXTURE_2D, intermediateTexture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_BUFFER, sobel_kernelTexture2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, targetTexture);

    glUseProgram(absValueProg);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    /**/
    GLenum e = glGetError();
}
