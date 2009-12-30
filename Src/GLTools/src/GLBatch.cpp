/*
GLBatch.cpp
 
Copyright (c) 2009, Richard S. Wright Jr.
GLTools Open Source Library
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list 
of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this list 
of conditions and the following disclaimer in the documentation and/or other 
materials provided with the distribution.

Neither the name of Richard S. Wright Jr. nor the names of other contributors may be used 
to endorse or promote products derived from this software without specific prior 
written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR 
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <GLBatch.h>
#include <GLShaderManager.h>

GLBatch::GLBatch(void): pVerts(NULL), pNorms(NULL), pColors(NULL), ppTexCoords(NULL), nVertsBuilding(0),
	nMaxVerts(0), nNumTextureUnits(0), bBatchDone(false)
	{
	
	}

GLBatch::~GLBatch(void)
	{
	delete [] pVerts;
	delete [] pNorms;
	delete [] pColors;
	for(GLuint i = 0; i < nNumTextureUnits; i++)
		delete [] ppTexCoords[i];
	delete [] ppTexCoords;
	}


// Just start over. No reallocations, etc.
void GLBatch::Reset(void)
	{
	nVertsBuilding = 0;
	}
	
void GLBatch::Begin(GLenum primitive, GLuint nVerts, GLuint nTextureUnits)
	{
	primitiveType = primitive;
	pVerts = new M3DVector3f[nVerts];
	nMaxVerts = nVerts;
	nNumTextureUnits = nTextureUnits;

	ppTexCoords = new M3DVector2f*[nNumTextureUnits];
	for(GLuint i = 0; i < nNumTextureUnits; i++)
		ppTexCoords[i] = new M3DVector2f[nMaxVerts];

	}
	
	
void GLBatch::End(void)
	{
	bBatchDone = true;
	}
        
void GLBatch::Vertex3f(GLfloat x, GLfloat y, GLfloat z)
	{
	// Ignore if we go past the end
	if(nVertsBuilding >= nMaxVerts)
		return;
	
	// Copy it in...
	pVerts[nVertsBuilding][0] = x;
	pVerts[nVertsBuilding][1] = y;
	pVerts[nVertsBuilding][2] = z;
	nVertsBuilding++;
	}
        
void GLBatch::Vertex3fv(M3DVector3f vVertex)
	{
	// Ignore if we go past the end
	if(nVertsBuilding >= nMaxVerts)
		return;
	
	// Copy it in...
	memcpy(pVerts[nVertsBuilding], vVertex, sizeof(M3DVector3f));
	nVertsBuilding++;	
	}
        
// Unlike normal OpenGL immediate mode, you must specify a normal per vertex
// or you will get junk...
void GLBatch::Normal3f(GLfloat x, GLfloat y, GLfloat z)
	{
	if(pNorms == NULL)
		pNorms = new M3DVector3f[nMaxVerts];
	
	// Ignore if we go past the end
	if(nVertsBuilding >= nMaxVerts)
		return;

	pNorms[nVertsBuilding][0] = x;
	pNorms[nVertsBuilding][1] = y;
	pNorms[nVertsBuilding][2] = z;
	}
        
// Ditto above
void GLBatch::Normal3fv(M3DVector3f vNormal)
	{
	if(pNorms == NULL)
		pNorms = new M3DVector3f[nMaxVerts];
	
	// Ignore if we go past the end
	if(nVertsBuilding >= nMaxVerts)
		return;

	memcpy(pNorms[nVertsBuilding], vNormal, sizeof(M3DVector3f));
	}
	

void GLBatch::Color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
	{
	if(pColors == NULL)
		pColors = new M3DVector4f[nMaxVerts];
	
	// Ignore if we go past the end
	if(nVertsBuilding >= nMaxVerts)
		return;

	pColors[nVertsBuilding][0] = r;
	pColors[nVertsBuilding][1] = g;
	pColors[nVertsBuilding][2] = b;
	pColors[nVertsBuilding][3] = a;	
	}
	
void GLBatch::Color4fv(M3DVector4f vColor)
	{
	if(pColors == NULL)
		pColors = new M3DVector4f[nMaxVerts];
	
	// Ignore if we go past the end
	if(nVertsBuilding >= nMaxVerts)
		return;

	memcpy(pColors[nVertsBuilding], vColor, sizeof(M3DVector4f));
	}
        
// Unlike normal OpenGL immediate mode, you must specify a texture coord
// per vertex or you will get junk...
void GLBatch::MultiTexCoord2f(GLuint texture, GLfloat s, GLfloat t)
	{
	// Ignore if we go past the end
	if(nVertsBuilding >= nMaxVerts)
		return;

	ppTexCoords[texture][nVertsBuilding][0] = s;
	ppTexCoords[texture][nVertsBuilding][1] = t;
	}
   
// Ditto above  
void GLBatch::MultiTexCoord2fv(GLuint texture, M3DVector2f vTexCoord)
	{	
	// Ignore if we go past the end
	if(nVertsBuilding >= nMaxVerts)
		return;

	memcpy(ppTexCoords[texture][nVertsBuilding], vTexCoord, sizeof(M3DVector2f));
	}


void GLBatch::Draw(bool bEnable)
	{
	if(!bBatchDone)
		return;
		
	// Where are the vertexes
	if(bEnable) glEnableVertexAttribArray(GLT_ATTRIBUTE_VERTEX);
	glVertexAttribPointer(GLT_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, pVerts);
	// Where are the Normals
	if(pNorms) {
		if(bEnable) glEnableVertexAttribArray(GLT_ATTRIBUTE_NORMAL);
		glVertexAttribPointer(GLT_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, pNorms);
		}
		
	// Where are the colors
	if(pColors) {
		if(bEnable) glEnableVertexAttribArray(GLT_ATTRIBUTE_COLOR);
		glVertexAttribPointer(GLT_ATTRIBUTE_COLOR, 4, GL_FLOAT, GL_FALSE, 0, pColors);
		}
	
	// Where are the texture coordinates
	for(GLuint i = 0; i < nNumTextureUnits; i++)
		{
		if(bEnable) glEnableVertexAttribArray(GLT_ATTRIBUTE_TEXTURE0+i);
		glVertexAttribPointer(GLT_ATTRIBUTE_TEXTURE0+i, 2, GL_FLOAT, GL_FALSE, 0, ppTexCoords[i]);
		}

	glDrawArrays(primitiveType, 0, nVertsBuilding);
	
	if(bEnable) {
		glDisableVertexAttribArray(GLT_ATTRIBUTE_VERTEX);
		glDisableVertexAttribArray(GLT_ATTRIBUTE_NORMAL);
		glDisableVertexAttribArray(GLT_ATTRIBUTE_COLOR);
		for(GLuint i = 0; i < nNumTextureUnits; i++)
			{
			glDisableVertexAttribArray(GLT_ATTRIBUTE_TEXTURE0+i);
			}
		}
	}
        
M3DVector2f* GLBatch::GetTextureCoords(GLuint iTextureUnit)
	{
	if(iTextureUnit > nNumTextureUnits)
		return NULL;
	
	return ppTexCoords[iTextureUnit];
	}
