// pixbuffs.h


#ifndef __FBOTEXTURES__
#define __FBOTEXTURES__

#include <gltools.h>
#include <GLShaderManager.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>

class PixBuffs
	{
	public:
		PixBuffs();
		virtual ~PixBuffs() {};

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
		
		bool UsingPBOs() { return bUsePBOPath; }

	protected:
		GLsizei	 screenWidth;			// Desired window or desktop width
		GLsizei  screenHeight;			// Desired window or desktop height
		
		GLboolean bFullScreen;			// Request to run full screen
		GLboolean bAnimated;			// Request for continual updates


		GLShaderManager		shaderManager;			// Shader Manager
		GLMatrixStack		modelViewMatrix;		// Modelview Matrix
		GLMatrixStack		projectionMatrix;		// Projection Matrix
		M3DMatrix44f        orthoMatrix;     
		GLFrustum			viewFrustum;			// View Frustum
		GLGeometryTransform	transformPipeline;		// Geometry Transform Pipeline
		GLFrame				cameraFrame;			// Camera frame

		GLTriangleBatch		torusBatch;
		GLBatch				floorBatch;
        GLBatch             screenQuad;

		GLuint				textures[1];
		GLuint				blurTextures[6];
		GLuint				pixBuffObjs[1];
		GLuint				curBlurTarget;
		bool				bUsePBOPath;
		GLfloat				speedFactor;
		GLuint				blurProg;
		void				*pixelData;
		GLuint				pixelDataSize;

		void MoveCamera(void);
		void DrawWorld(GLfloat yRot, GLfloat xPos);
		bool LoadBMPTexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode);

		void SetupBlurProg(void);
		
		// returns 1 - 6 for blur texture units
		// curPixBuf is always between 0 and 5
		void AdvanceBlurTaget() { curBlurTarget = ((curBlurTarget+ 1) %6); }
		GLuint GetBlurTarget0(){ return (1 + ((curBlurTarget + 5) %6)); }
		GLuint GetBlurTarget1(){ return (1 + ((curBlurTarget + 4) %6)); }
		GLuint GetBlurTarget2(){ return (1 + ((curBlurTarget + 3) %6)); }
		GLuint GetBlurTarget3(){ return (1 + ((curBlurTarget + 2) %6)); }
		GLuint GetBlurTarget4(){ return (1 + ((curBlurTarget + 1) %6)); }
		GLuint GetBlurTarget5(){ return (1 + ((curBlurTarget) %6)); }
	};
		
#endif // pixbuffs
