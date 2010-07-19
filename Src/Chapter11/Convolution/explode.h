// hdr_bloom.h


#ifndef __INSTANCING__
#define __INSTANCING__

#include <gltools.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>

class ConvolutionApp
    {
    public:
        ConvolutionApp();
        virtual ~ConvolutionApp() {};

        void Initialize(void);                            // Called after context is created
        void Shutdown(void);                            // Called before context is destroyed
        void Resize(GLsizei nWidth, GLsizei nHeight);    // Called when window is resized, at least once when window is created
        void Render(void);                                // Called to update OpenGL view
        
        // These methods are used by the calling framework. Set the appropriate internal
        // protected variables to tell the parent framework your desired configuration
        inline GLuint GetWidth(void) { return screenWidth; }
        inline GLuint GetHeight(void) { return screenHeight; }
        inline GLboolean GetFullScreen(void) { return bFullScreen; }
        inline GLboolean GetAnimated(void) { return GL_FALSE; }

    protected:
        GLsizei     screenWidth;            // Desired window or desktop width
        GLsizei     screenHeight;            // Desired window or desktop height

        GLboolean bFullScreen;            // Request to run full screen

        GLMatrixStack        modelViewMatrix;        // Modelview Matrix
        GLMatrixStack        projectionMatrix;        // Projection Matrix
        GLFrustum            viewFrustum;            // View Frustum
        GLGeometryTransform    transformPipeline;        // Geometry Transform Pipeline
        GLFrame                cameraFrame;            // Camera frame
        GLBatch             screenQuad;
        M3DMatrix44f        orthoMatrix;  

        GLuint                instancingProg;

        GLuint              absValueProg;

        GLuint              square_vao;
        GLuint              square_vbo;

        GLuint              sourceTexture;
        GLuint              gaussian_kernelTexture;
        GLuint              gaussian_kernelBuffer;

        GLuint              sobel_kernelTexture1;
        GLuint              sobel_kernelTexture2;
        GLuint              sobel_kernelBuffer1;
        GLuint              sobel_kernelBuffer2;

        GLuint              intermediateFBO;
        GLuint              intermediateTexture;

        GLuint              targetFBO;
        GLuint              targetTexture;

        GLfloat             kernelScale[2];

        void UpdateMode(void);
        void GenerateOrtho2DMat(GLuint imageWidth, GLuint imageHeight);
        bool LoadBMPTexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode);
        bool LoadOpenEXRImage(char *fileName, GLint textureName, GLuint &texWidth, GLuint &texHeight);
        void GenTexCoordOffsets(GLuint width, GLuint height);
        void SetupTexReplaceProg(GLfloat *vLightPos, GLfloat *vColor);
        void SetupFlatColorProg(GLfloat *vLightPos, GLfloat *vColor);
        void SetupHDRProg();
        void SetupBlurProg();
    };
        
#endif // __INSTANCING__
