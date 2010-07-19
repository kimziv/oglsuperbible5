// sbmviewer.h


#ifndef __SBMVIEWER__
#define __SBMVIEWER__

#include <gltools.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>

class SBMViewer
    {
    public:
        SBMViewer();
        virtual ~SBMViewer() {};

        void Initialize(void);                            // Called after context is created
        void Shutdown(void);                            // Called before context is destroyed
        void Resize(GLsizei nWidth, GLsizei nHeight);    // Called when window is resized, at least once when window is created
        void Render(void);                                // Called to update OpenGL view
        
        // These methods are used by the calling framework. Set the appropriate internal
        // protected variables to tell the parent framework your desired configuration
        inline GLuint GetWidth(void) { return screenWidth; }
        inline GLuint GetHeight(void) { return screenHeight; }
        inline GLboolean GetFullScreen(void) { return bFullScreen; }
        inline GLboolean GetAnimated(void) { return bAnimated; }

    protected:
        GLsizei     screenWidth;            // Desired window or desktop width
        GLsizei     screenHeight;            // Desired window or desktop height
        
        GLboolean bFullScreen;            // Request to run full screen
        GLboolean bAnimated;            // Request for continual updates

        GLMatrixStack        modelViewMatrix;        // Modelview Matrix
        GLMatrixStack        projectionMatrix;        // Projection Matrix
        GLFrustum            viewFrustum;            // View Frustum
        GLGeometryTransform    transformPipeline;        // Geometry Transform Pipeline
        GLFrame                cameraFrame;            // Camera frame
        GLBatch             screenQuad;
        M3DMatrix44f        orthoMatrix;  

        GLuint                instancingProg;

        GLuint              square_vao;
        GLuint              square_vbo;
        GLuint              element_buffer;

        GLuint              hdrFBO[1];
        GLuint              brightPassFBO[4];
        GLuint                textures[1];
        GLuint                hdrTextures[1];
        GLuint                brightBlurTextures[5];
        GLuint                windowTexture;
        GLfloat                exposure;
        GLfloat                bloomLevel;
        GLfloat                texCoordOffsets[5*5*2];
    };
        
#endif // __SBMVIEWER__
