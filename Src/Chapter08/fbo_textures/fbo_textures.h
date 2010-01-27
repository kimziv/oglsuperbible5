// fbo_textures.h


#ifndef __FBOTEXTURES__
#define __FBOTEXTURES__

#include <gltools.h>
#include <GLShaderManager.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>

class FBOtextures
	{
	public:
		FBOtextures();
		virtual ~FBOtextures() {};

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


		GLShaderManager		shaderManager;			// Shader Manager
		GLMatrixStack		modelViewMatrix;		// Modelview Matrix
		GLMatrixStack		projectionMatrix;		// Projection Matrix
		GLFrustum			viewFrustum;			// View Frustum
		GLGeometryTransform	transformPipeline;		// Geometry Transform Pipeline
		GLFrame				cameraFrame;			// Camera frame
		GLFrame				mirrorFrame;			// Mirror frame

		GLTriangleBatch		torusBatch;
		GLTriangleBatch		sphereBatch;
		GLTriangleBatch		cylinderBatch;
		GLBatch				floorBatch;
		GLBatch				mirrorBatch;
		GLBatch				mirrorBorderBatch;

		GLuint              fboName;
		GLuint				textures[1];
		GLuint				mirrorTexture;
        GLuint              depthBufferName; 

		void MoveCamera(void);
		void DrawWorld(GLfloat yRot);
		bool LoadBMPTexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode);

	};
		
#endif // __FBOTEXTURES__
