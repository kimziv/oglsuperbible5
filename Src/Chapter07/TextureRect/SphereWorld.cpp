// SphereWorld.cpp
// OpenGL SuperBible
// New and improved sphere world, this time with a texture rect
// Program by Richard S. Wright Jr.

#include <GLTools.h>
#include <GLShaderManager.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <Stopwatch.h>

#include <math.h>
#include <stdio.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <gl/glut.h>
#endif

#define NUM_SPHERES 50
GLFrame spheres[NUM_SPHERES];

GLShaderManager		shaderManager;			// Shader Manager
GLMatrixStack		modelViewMatrix;		// Modelview Matrix
GLMatrixStack		projectionMatrix;		// Projection Matrix
GLFrustum			viewFrustum;			// View Frustum
GLGeometryTransform	transformPipeline;		// Geometry Transform Pipeline
GLFrame				cameraFrame;			// Camera frame

GLTriangleBatch		torusBatch;
GLTriangleBatch		sphereBatch;
GLBatch				floorBatch;
GLBatch				logoBatch;

GLuint				uiTextures[4];
GLint				rectReplaceShader;
GLint				locRectMVP;
GLint				locRectTexture;


void DrawSongAndDance(GLfloat yRot)		// Called to draw dancing objects
	{
	static GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	static GLfloat vLightPos[] = { 0.0f, 3.0f, 0.0f, 1.0f };
	
	// Get the light position in eye space
	M3DVector4f	vLightTransformed;
	M3DMatrix44f mCamera;
	modelViewMatrix.GetMatrix(mCamera);
	m3dTransformVector4(vLightTransformed, vLightPos, mCamera);
	
	// Draw the light source
	modelViewMatrix.PushMatrix();
	modelViewMatrix.Translatev(vLightPos);
	shaderManager.UseStockShader(GLT_SHADER_FLAT, 
								 transformPipeline.GetModelViewProjectionMatrix(),
								 vWhite);
	sphereBatch.Draw();
	modelViewMatrix.PopMatrix();
    
    glBindTexture(GL_TEXTURE_2D, uiTextures[2]);
    for(int i = 0; i < NUM_SPHERES; i++) {
        modelViewMatrix.PushMatrix();
        modelViewMatrix.MultMatrix(spheres[i]);
        shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
                                     modelViewMatrix.GetMatrix(),
                                     transformPipeline.GetProjectionMatrix(),
                                     vLightTransformed, 
                                     vWhite,
                                     0);
        sphereBatch.Draw();
        modelViewMatrix.PopMatrix();
    }
	
	// Song and dance
	modelViewMatrix.Translate(0.0f, 0.2f, -2.5f);
	modelViewMatrix.PushMatrix();	// Saves the translated origin
	modelViewMatrix.Rotate(yRot, 0.0f, 1.0f, 0.0f);
	
	// Draw stuff relative to the camera
	glBindTexture(GL_TEXTURE_2D, uiTextures[1]);
	shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
								 modelViewMatrix.GetMatrix(),
								 transformPipeline.GetProjectionMatrix(),
								 vLightTransformed, 
								 vWhite,
								 0);
	torusBatch.Draw();
	modelViewMatrix.PopMatrix(); // Erased the rotate
	
	modelViewMatrix.Rotate(yRot * -2.0f, 0.0f, 1.0f, 0.0f);
	modelViewMatrix.Translate(0.8f, 0.0f, 0.0f);
	
	glBindTexture(GL_TEXTURE_2D, uiTextures[2]);
	shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
								 modelViewMatrix.GetMatrix(),
								 transformPipeline.GetProjectionMatrix(),
								 vLightTransformed, 
								 vWhite,
								 0);
	sphereBatch.Draw();
	}
	
	
bool LoadTGATexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
	{
	GLbyte *pBits;
	int nWidth, nHeight, nComponents;
	GLenum eFormat;
	
	// Read the texture bits
	pBits = gltReadTGABits(szFileName, &nWidth, &nHeight, &nComponents, &eFormat);
	if(pBits == NULL) 
		return false;
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
		
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB, nWidth, nHeight, 0,
				 eFormat, GL_UNSIGNED_BYTE, pBits);
	
    free(pBits);

    if(minFilter == GL_LINEAR_MIPMAP_LINEAR || 
       minFilter == GL_LINEAR_MIPMAP_NEAREST ||
       minFilter == GL_NEAREST_MIPMAP_LINEAR ||
       minFilter == GL_NEAREST_MIPMAP_NEAREST)
        glGenerateMipmap(GL_TEXTURE_2D);
            
	return true;
	}


bool LoadTGATextureRect(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
	{
	GLbyte *pBits;
	int nWidth, nHeight, nComponents;
	GLenum eFormat;
	
	// Read the texture bits
	pBits = gltReadTGABits(szFileName, &nWidth, &nHeight, &nComponents, &eFormat);
	if(pBits == NULL) 
		return false;
	
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, wrapMode);
	
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, magFilter);
		
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, nComponents, nWidth, nHeight, 0,
				 eFormat, GL_UNSIGNED_BYTE, pBits);
	
    free(pBits);

	return true;
	}

//////////////////////////////////////////////////////////////////
// This function does any needed initialization on the rendering
// context. 
void SetupRC()
    {
	// Make sure OpenGL entry points are set
	glewInit();
	
	// Initialze Shader Manager
	shaderManager.InitializeStockShaders();
	
	glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
	// This makes a torus
	gltMakeTorus(torusBatch, 0.4f, 0.15f, 40, 20);
	
	// This makes a sphere
	gltMakeSphere(sphereBatch, 0.1f, 26, 13);
	
	
	// Make the solid ground
	GLfloat texSize = 10.0f;
	floorBatch.Begin(GL_TRIANGLE_FAN, 4, 1);
	floorBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	floorBatch.Vertex3f(-20.0f, -0.41f, 20.0f);
	
	floorBatch.MultiTexCoord2f(0, texSize, 0.0f);
    floorBatch.Vertex3f(20.0f, -0.41f, 20.0f);
	
	floorBatch.MultiTexCoord2f(0, texSize, texSize);
	floorBatch.Vertex3f(20.0f, -0.41f, -20.0f);
	
	floorBatch.MultiTexCoord2f(0, 0.0f, texSize);
	floorBatch.Vertex3f(-20.0f, -0.41f, -20.0f);
	floorBatch.End();
	

	int x = 500;
	int y = 155;
	int width = 300;
	int height = 155;
	logoBatch.Begin(GL_TRIANGLE_FAN, 4, 1);
	
        // Upper left hand corner
        logoBatch.MultiTexCoord2f(0, 0.0f, height);
        logoBatch.Vertex3f(x, y, 0.0);
        
        // Lower left hand corner
        logoBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
        logoBatch.Vertex3f(x, y - height, 0.0f);

        // Lower right hand corner
        logoBatch.MultiTexCoord2f(0, width, 0.0f);
        logoBatch.Vertex3f(x + width, y - height, 0.0f);

        // Upper righ hand corner
        logoBatch.MultiTexCoord2f(0, width, height);
        logoBatch.Vertex3f(x + width, y, 0.0f);

	logoBatch.End();

	// Make 4 texture objects
	glGenTextures(4, uiTextures);
	
	// Load the Marble
	glBindTexture(GL_TEXTURE_2D, uiTextures[0]);
	LoadTGATexture("marble.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);
	
	// Load Mars
	glBindTexture(GL_TEXTURE_2D, uiTextures[1]);
	LoadTGATexture("marslike.tga", GL_LINEAR_MIPMAP_LINEAR, 
				   GL_LINEAR, GL_CLAMP_TO_EDGE);
	
	// Load Moon
	glBindTexture(GL_TEXTURE_2D, uiTextures[2]);
	LoadTGATexture("moonlike.tga", GL_LINEAR_MIPMAP_LINEAR,
				   GL_LINEAR, GL_CLAMP_TO_EDGE);

	// Load the Logo
	glBindTexture(GL_TEXTURE_RECTANGLE, uiTextures[3]);
	LoadTGATextureRect("OpenGL-Logo.tga", GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE);

    rectReplaceShader = gltLoadShaderPairWithAttributes("RectReplace.vp", "RectReplace.fp", 
			2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_TEXTURE0, "vTexCoord");
                 

	locRectMVP = glGetUniformLocation(rectReplaceShader, "mvpMatrix");
	locRectTexture = glGetUniformLocation(rectReplaceShader, "rectangleImage");


    // Randomly place the spheres
    for(int i = 0; i < NUM_SPHERES; i++) {
        GLfloat x = ((GLfloat)((rand() % 400) - 200) * 0.1f);
        GLfloat z = ((GLfloat)((rand() % 400) - 200) * 0.1f);
        spheres[i].SetOrigin(x, 0.0f, z);
        }
    }

////////////////////////////////////////////////////////////////////////
// Do shutdown for the rendering context
void ShutdownRC(void)
    {
    glDeleteTextures(3, uiTextures);
    }



        
// Called to draw scene
void RenderScene(void)
	{
	static CStopWatch	rotTimer;
	float yRot = rotTimer.GetElapsedSeconds() * 60.0f;
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	modelViewMatrix.PushMatrix();	
	M3DMatrix44f mCamera;
	cameraFrame.GetCameraMatrix(mCamera);
	modelViewMatrix.MultMatrix(mCamera);
	
	// Draw the world upside down
	modelViewMatrix.PushMatrix();
	modelViewMatrix.Scale(1.0f, -1.0f, 1.0f); // Flips the Y Axis
	modelViewMatrix.Translate(0.0f, 0.8f, 0.0f); // Scootch the world down a bit...
	glFrontFace(GL_CW);
	DrawSongAndDance(yRot);
	glFrontFace(GL_CCW);
	modelViewMatrix.PopMatrix();
	
	// Draw the solid ground
	glEnable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, uiTextures[0]);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	static GLfloat vFloorColor[] = { 1.0f, 1.0f, 1.0f, 0.75f};
	shaderManager.UseStockShader(GLT_SHADER_TEXTURE_MODULATE,
								 transformPipeline.GetModelViewProjectionMatrix(),
								 vFloorColor,
								 0);
	
	floorBatch.Draw();
	glDisable(GL_BLEND);
	
	
	DrawSongAndDance(yRot);
	
	modelViewMatrix.PopMatrix();

	// Render the overlay
    
    // Creating this matrix really doesn't need to be done every frame. I'll leave it here
    // so all the pertenant code is together
    M3DMatrix44f mScreenSpace;
    m3dMakeOrthographicMatrix(mScreenSpace, 0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f);
        
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	glUseProgram(rectReplaceShader);
	glUniform1i(locRectTexture, 0);
	glUniformMatrix4fv(locRectMVP, 1, GL_FALSE, mScreenSpace);
	glBindTexture(GL_TEXTURE_RECTANGLE, uiTextures[3]);
	logoBatch.Draw();
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
        
    // Do the buffer Swap
    glutSwapBuffers();
        
    // Do it again
    glutPostRedisplay();
    }



// Respond to arrow keys by moving the camera frame of reference
void SpecialKeys(int key, int x, int y)
    {
	float linear = 0.1f;
	float angular = float(m3dDegToRad(5.0f));
	
	if(key == GLUT_KEY_UP)
		cameraFrame.MoveForward(linear);
	
	if(key == GLUT_KEY_DOWN)
		cameraFrame.MoveForward(-linear);
	
	if(key == GLUT_KEY_LEFT)
		cameraFrame.RotateWorld(angular, 0.0f, 1.0f, 0.0f);
	
	if(key == GLUT_KEY_RIGHT)
		cameraFrame.RotateWorld(-angular, 0.0f, 1.0f, 0.0f);	
    }


void ChangeSize(int nWidth, int nHeight)
    {
	glViewport(0, 0, nWidth, nHeight);
	transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
	
	viewFrustum.SetPerspective(35.0f, float(nWidth)/float(nHeight), 1.0f, 100.0f);
	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	modelViewMatrix.LoadIdentity();
	}

int main(int argc, char* argv[])
    {
	gltSetWorkingDirectory(argv[0]);
		
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800,600);
  
    glutCreateWindow("OpenGL SphereWorld with Texture Rectangle");
 
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    glutSpecialFunc(SpecialKeys);

    SetupRC();
    glutMainLoop();    
    ShutdownRC();
    return 0;
    }
