/*
GLBatch.h
 
Copyright (c) 2009, Richard S. Wright Jr.
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

#ifndef __GL_BATCH__
#define __GL_BATCH__

#include <GLTools.h>
#include <math3d.h>
#include <GLBatchBase.h>


class GLBatch : public GLBatchBase
    {
    public:
        GLBatch(void);
        virtual ~GLBatch(void);
        
        void Begin(GLenum primitive, GLuint nVerts, GLuint nTextureUnits = 0);
        void End(void);
        
        void Reset(void);
        
        void Vertex3f(GLfloat x, GLfloat y, GLfloat z);
        void Vertex3fv(M3DVector3f vVertex);
        
        void Normal3f(GLfloat x, GLfloat y, GLfloat z);
        void Normal3fv(M3DVector3f vNormal);
        
        void Color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
        void Color4fv(M3DVector4f vColor);
        
        void MultiTexCoord2f(GLuint texture, GLfloat s, GLfloat t);
        void MultiTexCoord2fv(GLuint texture, M3DVector2f vTexCoord);
               
        // Draw - make sure you call glEnableClientState for these
        // arrays yourself if bEnable is false.
        virtual void Draw(bool bEnable = true);
        
        // Query functions
        inline bool IsStatic(void) { return pVerts == NULL; }
        inline bool HasTexture(void) { return ppTexCoords == NULL; }
        inline bool HasNormals(void) { return pNorms == NULL; }
        inline bool IsComplete(void) { return bBatchDone; }
        inline GLuint GetTextureUnitCount(void) { return nNumTextureUnits; }
        inline GLuint GetVertexCount(void) { return nVertsBuilding; }
        inline GLenum GetPrimitive(void) { return primitiveType; }
       
        // These return NULL if the batch is marked as static
        inline M3DVector3f* GetVertexArray(void) { return pVerts; }
        inline M3DVector3f* GetNormalArray(void) { return pNorms; }
        inline M3DVector4f* GetColorArray(void) { return pColors; }
        M3DVector2f* GetTextureCoords(GLuint iTextureUnit);
        
        
        
    protected:
		GLenum		primitiveType;		// What am I drawing....
        M3DVector3f *pVerts;			// Array of vertices
        M3DVector3f *pNorms;			// Array of normals
        M3DVector4f *pColors;			// Array of colors
        M3DVector2f **ppTexCoords;		// Multiple Arrays of texture coordinates
        
        GLuint nVertsBuilding;			// Building up vertexes
        GLuint nMaxVerts;				// Amount of vertex data initially allocated
        GLuint nNumTextureUnits;		// Number of texture coordinate sets
        
        bool	bBatchDone;				// Batch has been built
    };

#endif // __GL_BATCH__