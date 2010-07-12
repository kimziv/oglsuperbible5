#ifndef __SBM_H__
#define __SBM_H__

#include <gltools.h>

typedef struct SBM_HEADER_t
{
    unsigned int magic;
    unsigned int size;
    char name[64];
    unsigned int num_attribs;
    unsigned int num_frames;
    unsigned int num_vertices;
    unsigned int num_indices;
    unsigned int index_type;
} SBM_HEADER;

typedef struct SBM_ATTRIB_HEADER_t
{
    char name[64];
    unsigned int type;
    unsigned int components;
    unsigned int flags;
} SBM_ATTRIB_HEADER;

typedef struct SBM_FRAME_HEADER_t
{
    unsigned int first;
    unsigned int count;
    unsigned int flags;
} SBM_FRAME_HEADER;

typedef struct SBM_VEC4F_t
{
    float x;
    float y;
    float z;
    float w;
} SBM_VEC4F;

class SBObject
{
public:
    SBObject(void);
    virtual ~SBObject(void);

    bool LoadFromSBM(const char * filename, int vertexIndex, int normalIndex, int texCoord0Index);
    void Render(unsigned int frame_index = 0, unsigned int instances = 0);
    bool Free(void);

    unsigned int GetAttributeCount(void) const
    {
        return m_header.num_attribs;
    }

    const char * GetAttributeName(unsigned int index) const
    {
        return index < m_header.num_attribs ? m_attrib[index].name : 0;
    }

protected:
    GLuint m_vao;
    GLuint m_attribute_buffer;
    GLuint m_index_buffer;

    SBM_HEADER m_header;
    SBM_ATTRIB_HEADER * m_attrib;
    SBM_FRAME_HEADER * m_frame;
};

#endif /* __SBM_H__ */
