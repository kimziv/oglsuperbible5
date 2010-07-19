#include "sbmviewer.h"

typedef struct SBM_HEADER_t
{
    unsigned int magic;
    char name[64];
    unsigned int num_attribs;
    unsigned int num_frames;
    unsigned int num_vertices;
    unsigned int num_indices;
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

    bool LoadFromSBM(const char * filename);
    void Render(unsigned int frame_index = 0);
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

SBObject::SBObject(void)
    : m_vao(0),
      m_attribute_buffer(0),
      m_index_buffer(0),
      m_attrib(0),
      m_frame(0)
{

}

SBObject::~SBObject(void)
{
    Free();
}

bool SBObject::LoadFromSBM(const char * filename)
{
    FILE * f = NULL;

    f = fopen(filename, "rb");

    fseek(f, 0, SEEK_END);
    size_t filesize = ftell(f);
    fseek(f, 0, SEEK_SET);

    unsigned char * data = new unsigned char [filesize];
    unsigned char * raw_data;
    fread(data, filesize, 1, f);
    fclose(f);

    SBM_HEADER * header = (SBM_HEADER *)data;
    raw_data = data + sizeof(SBM_HEADER) + header->num_attribs * sizeof(SBM_ATTRIB_HEADER) + header->num_frames * sizeof(SBM_FRAME_HEADER);
    SBM_ATTRIB_HEADER * attrib_header = (SBM_ATTRIB_HEADER *)(data + sizeof(SBM_HEADER));
    SBM_FRAME_HEADER * frame_header = (SBM_FRAME_HEADER *)(data + sizeof(SBM_HEADER) + header->num_attribs * sizeof(SBM_ATTRIB_HEADER));
    unsigned int total_data_size = 0;

    memcpy(&m_header, header, sizeof(SBM_HEADER));
    m_attrib = new SBM_ATTRIB_HEADER[header->num_attribs];
    memcpy(m_attrib, attrib_header, header->num_attribs * sizeof(SBM_ATTRIB_HEADER));
    m_frame = new SBM_FRAME_HEADER[header->num_frames];
    memcpy(m_frame, frame_header, header->num_frames * sizeof(SBM_FRAME_HEADER));

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    glGenBuffers(1, &m_attribute_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_attribute_buffer);
    glGenBuffers(1, &m_index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);

    unsigned int i;

    for (i = 0; i < header->num_attribs; i++) {
        glVertexAttribPointer(i, m_attrib[i].components, m_attrib[i].type, GL_FALSE, 0, (GLvoid *)total_data_size);
        glEnableVertexAttribArray(i);
        total_data_size += m_attrib[i].components * sizeof(GLfloat) * header->num_vertices;
    }

    glBufferData(GL_ARRAY_BUFFER, total_data_size, raw_data, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, header->num_indices * sizeof(GLuint), raw_data + total_data_size, GL_STATIC_DRAW);

    glBindVertexArray(0);

    delete [] data;

    return true;
}

bool SBObject::Free(void)
{
    glDeleteBuffers(1, &m_index_buffer);
    glDeleteBuffers(1, &m_attribute_buffer);
    glDeleteVertexArrays(1, &m_vao);

    delete [] m_attrib;
    m_attrib = NULL;

    delete [] m_frame;
    m_frame = NULL;

    return true;
}

void SBObject::Render(unsigned int frame_index)
{
    if (frame_index >= m_header.num_frames)
        return;

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_frame[frame_index].count, GL_UNSIGNED_INT, (GLvoid *)(m_frame[frame_index].first * sizeof(GLuint)));
    glBindVertexArray(0);
}

SBMViewer::SBMViewer(void)
{

}

unsigned int indices_start;
unsigned int index_count;

SBObject * object;

void SBMViewer::Initialize(void)
{
    unsigned int e;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }

    instancingProg = gltLoadShaderPair("sbmviewer.vs", "sbmviewer.fs");

    object = new SBObject;
    object->LoadFromSBM("C:\\temp\\teapot.sbm");

    unsigned int i;

    for (i = 0; i < object->GetAttributeCount(); i++) {
        glBindAttribLocation(instancingProg, i, object->GetAttributeName(i));
        e = glGetError();
    }

    glLinkProgram(instancingProg);
    glUseProgram(instancingProg);
}

void SBMViewer::Shutdown(void)
{
    delete object;
}

void SBMViewer::Resize(GLsizei nWidth, GLsizei nHeight)
{
    screenWidth = nWidth;
    screenHeight = nHeight;
    glViewport(0, 0, nWidth, nHeight);
}

void SBMViewer::Render(void)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glUseProgram(instancingProg);

    M3DMatrix44f matrix;
    m3dMakePerspectiveMatrix(matrix, 60.0f, (float)screenWidth / (float)screenHeight, 0.1f, 1000.0f);
    glUniformMatrix4fv(0, 1, GL_FALSE, matrix);

    object->Render(0);

    unsigned int e = glGetError();
}
