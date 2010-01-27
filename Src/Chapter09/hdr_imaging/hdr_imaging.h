// hdr_imaging.h


#ifndef __HDRIMAGING__
#define __HDRIMAGING__

#include <gltools.h>
#include <GLShaderManager.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>

class HDRImaging
	{
	public:
		HDRImaging();
		virtual ~HDRImaging() {};

		void Initialize(void);							// Called after context is created
		void Shutdown(void);							// Called before context is destroyed
		void Resize(GLsizei nWidth, GLsizei nHeight);	// Called when window is resized, at least once when window is created
		void Render(void);								// Called to update OpenGL view
		void DrawToScreen(void);
		
		// These methods are used by the calling framework. Set the appropriate internal
		// protected variables to tell the parent framework your desired configuration
		inline GLuint GetWidth(void) { return screenWidth; }
		inline GLuint GetHeight(void) { return screenHeight; }
		inline GLboolean GetFullScreen(void) { return bFullScreen; }
		inline GLboolean GetAnimated(void) { return bAnimated; }
			
			
	protected:
		GLsizei	 screenWidth;			// Desired window or desktop width
		GLsizei  screenHeight;			// Desired window or desktop height
		
		GLboolean bFullScreen;			// Request to run full screen
		GLboolean bAnimated;			// Request for continual updates

		GLMatrixStack		modelViewMatrix;		// Modelview Matrix
		GLMatrixStack		projectionMatrix;		// Projection Matrix
		GLGeometryTransform	transformPipeline;		// Geometry Transform Pipeline
        GLBatch             screenQuad;
		GLBatch             fboQuad;
        M3DMatrix44f        orthoMatrix;  
		M3DMatrix44f        fboOrthoMatrix; 

		GLuint				hdrTextures[1];
		GLuint				lutTxtures[1];
		GLuint				fboTextures[1];
		GLuint				hdrTexturesWidth[1];
		GLuint				hdrTexturesHeight[1];
		GLuint				curHDRTex;
		GLuint				fboName;
        GLuint              mapTexProg;
		GLuint              varExposureProg;
		GLuint              adaptiveProg;
		GLuint              curProg;
		GLfloat				exposure;

		void UpdateMode(void);
        void GenerateOrtho2DMat(GLuint windowWidth, GLuint windowHeight, GLuint imageWidth, GLuint imageHeight);
		void GenerateFBOOrtho2DMat(GLuint imageWidth, GLuint imageHeight);
        void SetupHDRProg();
		void SetupStraightTexProg();
        bool LoadOpenEXRImage(char *fileName, GLint textureName, GLuint &texWidth, GLuint &texHeight);
	};
		
#endif // __HDRIMAGING__
