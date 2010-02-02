// Primitieves.cpp
// OpenGL SuperBible, Chapter 2
// Demonstrates the 7 Geometric Primitives
// Program by Richard S. Wright Jr.

#include <GLTools.h>	// OpenGL toolkit
#include <GLMatrixStack.h>
#include <GLFrame.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLGeometryTransform.h>

#include <math.h>
#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

/////////////////////////////////////////////////////////////////////////////////
// An assortment of needed classes
GLShaderManager		shaderManager;
GLMatrixStack		modelViewMatrix;
GLMatrixStack		projectionMatrix;
GLFrame				cameraFrame;
GLFrame             objectFrame;
GLFrustum			viewFrustum;

GLBatch				pointBatch;
GLBatch				lineBatch;
GLBatch				lineStripBatch;
GLBatch				lineLoopBatch;
GLBatch				triangleBatch;
GLBatch             triangleStripBatch;
GLBatch             triangleFanBatch;

GLGeometryTransform	transformPipeline;
M3DMatrix44f		shadowMatrix;


// Keep track of effects step
int nStep = 0;


///////////////////////////////////////////////////////////////////////////////
// This function does any needed initialization on the rendering context. 
// This is the first opportunity to do any OpenGL related tasks.
void SetupRC()
	{
    // Black background
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f );

	shaderManager.InitializeStockShaders();

	glEnable(GL_DEPTH_TEST);

	transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);

	cameraFrame.MoveForward(-15.0f);

    //////////////////////////////////////////////////////////////////////
    // Okay, build the batches. First fill a batch with points
    // in a spiral pattern
    GLfloat vPoints[200][3];
    GLfloat radius = 1.0f;
    GLfloat z = -2.35f;
    int iCounter = 0;
    for(GLfloat angle = 0.0f; angle <= (2.0f*M3D_PI)*3.0f; angle += 0.3f)
		{
        GLfloat x = radius * sin(angle);
        GLfloat y = radius * cos(angle);
            
        // Specify the point and move the Z value up a little	
        z += 0.075f;
        vPoints[iCounter][0] = x;
        vPoints[iCounter][1] = y;
        vPoints[iCounter][2] = z;
        iCounter++;
		}

    // Load point batch
    pointBatch.Begin(GL_POINTS, iCounter);
    pointBatch.CopyVertexData3f(vPoints);
    pointBatch.End();
    
    // Load as a bunch of line segments
    lineBatch.Begin(GL_LINES, iCounter);
    lineBatch.CopyVertexData3f(vPoints);
    lineBatch.End();
    
    // Load as a single line segment
    lineStripBatch.Begin(GL_LINE_STRIP, iCounter);
    lineStripBatch.CopyVertexData3f(vPoints);
    lineStripBatch.End();
    
    // Single line, connect first and last points
    lineLoopBatch.Begin(GL_LINE_LOOP, iCounter);
    lineLoopBatch.CopyVertexData3f(vPoints);
    lineLoopBatch.End();
    
    //////////////////////////////////////////////////////////////////////
    // For Triangles, we need to be a little more creative

    // For Triangles, we'll make a Pyramid
    GLfloat vPyramid[12][3] = { -2.0f, 0.0f, -2.0f, 
                                2.0f, 0.0f, -2.0f, 
                                0.0f, 4.0f, 0.0f,
                                
                                2.0f, 0.0f, -2.0f,
                                2.0f, 0.0f, 2.0f,
                                0.0f, 4.0f, 0.0f,
                                
                                2.0f, 0.0f, 2.0f,
                                -2.0f, 0.0f, 2.0f,
                                0.0f, 4.0f, 0.0f,
                                
                                -2.0f, 0.0f, 2.0f,
                                -2.0f, 0.0f, -2.0f,
                                 0.0f, 4.0f, 0.0f};
    
    triangleBatch.Begin(GL_TRIANGLES, 12);
    triangleBatch.CopyVertexData3f(vPyramid);
    triangleBatch.End();
    

    // For a Triangle fan, just a 6 sided hex. Raise the center up a bit
    int nVerts = 0;
    GLfloat r = 3.0f;
    vPoints[nVerts][0] = 0.0f;
    vPoints[nVerts][1] = 0.0f;
    vPoints[nVerts][2] = 0.0f;

    for(GLfloat angle = 0; angle < M3D_2PI; angle += M3D_2PI / 6.0f) {
        nVerts++;
        vPoints[nVerts][0] = float(cos(angle)) * r;
        vPoints[nVerts][1] = float(sin(angle)) * r;
        vPoints[nVerts][2] = -0.5f;
        }

    nVerts++;
    vPoints[nVerts][0] = r;
    vPoints[nVerts][1] = 0;
    vPoints[nVerts][2] = 0.0f;
        
    triangleFanBatch.Begin(GL_TRIANGLE_FAN, 8);
    triangleFanBatch.CopyVertexData3f(vPoints);
    triangleFanBatch.End();     
        
    
    // For triangle strips, a little patch of terrain maybe
    iCounter = 0;
    radius = 3.0f;
    for(GLfloat angle = 0.0f; angle <= (2.0f*M3D_PI); angle += 0.3f)
		{
        GLfloat x = radius * sin(angle);
        GLfloat y = radius * cos(angle);
            
        // Specify the point and move the Z value up a little	
        vPoints[iCounter][0] = x;
        vPoints[iCounter][1] = y;
        vPoints[iCounter][2] = -0.5;
        iCounter++;

        vPoints[iCounter][0] = x;
        vPoints[iCounter][1] = y;
        vPoints[iCounter][2] = 0.5;
        iCounter++;            
		}

    // Close up the loop
    vPoints[iCounter][0] = vPoints[0][0];
    vPoints[iCounter][1] = vPoints[0][1];
    vPoints[iCounter][2] = -0.5;
    iCounter++;
    
    vPoints[iCounter][0] = vPoints[1][0];
    vPoints[iCounter][1] = vPoints[1][1];
    vPoints[iCounter][2] = 0.5;
    iCounter++;            
        
        
        
    triangleStripBatch.Begin(GL_TRIANGLE_STRIP, iCounter);
    triangleStripBatch.CopyVertexData3f(vPoints);
    triangleStripBatch.End();    
    }



///////////////////////////////////////////////////////////////////////////////
// Called to draw scene
void RenderScene(void)
	{
    static GLfloat rotValue = 0.0f;

    GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
    GLfloat vBlack[] = { 0.0f, 0.0f, 0.0f, 1.0f };

    
	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	modelViewMatrix.PushMatrix();
		M3DMatrix44f mCamera;
		cameraFrame.GetCameraMatrix(mCamera);
		modelViewMatrix.MultMatrix(mCamera);

        M3DMatrix44f mObjectFrame;
        objectFrame.GetMatrix(mObjectFrame);
        modelViewMatrix.MultMatrix(mObjectFrame);

        shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlack);

        switch(nStep) {
            case 0:
                glPointSize(4.0f);
                pointBatch.Draw();
                glPointSize(1.0f);
                break;
            case 1:
                glLineWidth(2.0f);
                lineBatch.Draw();
                glLineWidth(1.0f);
                break;
            case 2:
                glLineWidth(2.0f);
                lineStripBatch.Draw();
                glLineWidth(1.0f);
                break;
            case 3:
                glLineWidth(2.0f);
                lineLoopBatch.Draw();
                glLineWidth(1.0f);
                break;
            case 4:
                shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
                triangleBatch.Draw();
 
                // Draw black outline
                glPolygonOffset(-1.0f, -1.0f);
                glEnable(GL_LINE_SMOOTH);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glEnable(GL_POLYGON_OFFSET_LINE);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glLineWidth(2.5f);
                shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlack);
                triangleBatch.Draw();
                
                // Restore polygon mode and depht testing
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glDisable(GL_POLYGON_OFFSET_LINE);
                glLineWidth(1.0f);
                glDisable(GL_BLEND);
                glDisable(GL_LINE_SMOOTH);
                
                break;
            case 5:
                shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
                triangleStripBatch.Draw();
                glPolygonOffset(-1.0f, -3.0f);
                glEnable(GL_LINE_SMOOTH);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glEnable(GL_POLYGON_OFFSET_LINE);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glLineWidth(2.5f);
                shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlack);
                triangleStripBatch.Draw();
                
                // Restore polygon mode and depht testing
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glDisable(GL_POLYGON_OFFSET_LINE);
                glLineWidth(1.0f);
                glDisable(GL_BLEND);
                glDisable(GL_LINE_SMOOTH);
                break;
            case 6:
                shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
                // Draw normally
                triangleFanBatch.Draw();

                // Draw black outline
                glPolygonOffset(-1.0f, -1.0f);
                glEnable(GL_LINE_SMOOTH);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glEnable(GL_POLYGON_OFFSET_LINE);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glLineWidth(2.5f);
                shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlack);
                triangleFanBatch.Draw();
                
                // Restore polygon mode and depht testing
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glDisable(GL_POLYGON_OFFSET_LINE);
                glLineWidth(1.0f);
                glDisable(GL_BLEND);
                glDisable(GL_LINE_SMOOTH);
                break;
            }
		
	modelViewMatrix.PopMatrix();

    rotValue += 2.0f;

	// Flush drawing commands
	glutSwapBuffers();
    }


// Respond to arrow keys by moving the camera frame of reference
void SpecialKeys(int key, int x, int y)
    {
	if(key == GLUT_KEY_UP)
		objectFrame.RotateWorld(m3dDegToRad(-5.0f), 1.0f, 0.0f, 0.0f);
    
	if(key == GLUT_KEY_DOWN)
		objectFrame.RotateWorld(m3dDegToRad(5.0f), 1.0f, 0.0f, 0.0f);
	
	if(key == GLUT_KEY_LEFT)
		objectFrame.RotateWorld(m3dDegToRad(-5.0f), 0.0f, 1.0f, 0.0f);
    
	if(key == GLUT_KEY_RIGHT)
		objectFrame.RotateWorld(m3dDegToRad(5.0f), 0.0f, 1.0f, 0.0f);
    
	glutPostRedisplay();
    }




///////////////////////////////////////////////////////////////////////////////
// A normal ASCII key has been pressed.
// In this case, advance the scene when the space bar is pressed
void KeyPressFunc(unsigned char key, int x, int y)
	{
	if(key == 32)
		{
		nStep++;

		if(nStep > 6)
			nStep = 0;
		}
        
    switch(nStep)
        {
        case 0: 
            glutSetWindowTitle("GL_POINTS");
            break;
        case 1:
            glutSetWindowTitle("GL_LINES");
            break;
        case 2:
            glutSetWindowTitle("GL_LINE_STRIP");
            break;
        case 3:
            glutSetWindowTitle("GL_LINE_LOOP");
            break;
        case 4:
            glutSetWindowTitle("GL_TRIANGLES");
            break;
        case 5:
            glutSetWindowTitle("GL_TRIANGLE_STRIP");
            break;
        case 6:
        default:
            glutSetWindowTitle("GL_TRIANGLE_FAN");
            break;
        }
                
    glutPostRedisplay();
	}

///////////////////////////////////////////////////////////////////////////////
// Window has changed size, or has just been created. In either case, we need
// to use the window dimensions to set the viewport and the projection matrix.
void ChangeSize(int w, int h)
	{
	glViewport(0, 0, w, h);
	viewFrustum.SetPerspective(35.0f, float(w) / float(h), 1.0f, 500.0f);
	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	modelViewMatrix.LoadIdentity();
	}

///////////////////////////////////////////////////////////////////////////////
// Main entry point for GLUT based programs
int main(int argc, char* argv[])
	{
	gltSetWorkingDirectory(argv[0]);
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(800, 600);
	glutCreateWindow("OpenGL Primitive Sampler");
    glutReshapeFunc(ChangeSize);
    glutKeyboardFunc(KeyPressFunc);
    glutSpecialFunc(SpecialKeys);
    glutDisplayFunc(RenderScene);
        
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
		return 1;
		}
	

	SetupRC();

	glutMainLoop();
	return 0;
	}
