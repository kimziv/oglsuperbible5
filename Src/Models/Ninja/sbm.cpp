#include "sbm.h"

#include <gltools.h>

#include <stdio.h>

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

bool SBObject::LoadFromSBM(const char * filename, int vertexIndex, int normalIndex, int texCoord0Index)
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

    unsigned int i;

    for (i = 0; i < header->num_attribs; i++) {
        int attribIndex = i;

        if(attribIndex == 0)
            attribIndex = vertexIndex;
        else if(attribIndex == 1)
            attribIndex = normalIndex;
         else if(attribIndex == 2)
            attribIndex = texCoord0Index;

        glVertexAttribPointer(attribIndex, m_attrib[i].components, m_attrib[i].type, GL_FALSE, 0, (GLvoid *)total_data_size);
        glEnableVertexAttribArray(attribIndex);
        total_data_size += m_attrib[i].components * sizeof(GLfloat) * header->num_vertices;
    }

    glBufferData(GL_ARRAY_BUFFER, total_data_size, raw_data, GL_STATIC_DRAW);

    if (header->num_indices) {
        glGenBuffers(1, &m_index_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
        unsigned int element_size;
        switch (header->index_type) {
            case GL_UNSIGNED_SHORT:
                element_size = sizeof(GLushort);
                break;
            default:
                element_size = sizeof(GLuint);
                break;
        }
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, header->num_indices * element_size, data + total_data_size, GL_STATIC_DRAW);
    }

    glBindVertexArray(0);

    delete [] data;

    return true;
}

bool SBObject::Free(void)
{
    glDeleteBuffers(1, &m_index_buffer);
    m_index_buffer = 0;
    glDeleteBuffers(1, &m_attribute_buffer);
    m_attribute_buffer = 0;
    glDeleteVertexArrays(1, &m_vao);
    m_vao = 0;

    delete [] m_attrib;
    m_attrib = NULL;

    delete [] m_frame;
    m_frame = NULL;

    return true;
}

void SBObject::Render(unsigned int frame_index, unsigned int instances)
{
    if (frame_index >= m_header.num_frames)
        return;

    glBindVertexArray(m_vao);
    if (instances) {
        if (m_header.num_indices)
            glDrawElementsInstanced(GL_TRIANGLES, m_frame[frame_index].count, GL_UNSIGNED_INT, (GLvoid *)(m_frame[frame_index].first * sizeof(GLuint)), instances);
        else
            glDrawArraysInstanced(GL_TRIANGLES, m_frame[frame_index].first, m_frame[frame_index].count, instances);
    } else {
        if (m_header.num_indices)
            glDrawElements(GL_TRIANGLES, m_frame[frame_index].count, GL_UNSIGNED_INT, (GLvoid *)(m_frame[frame_index].first * sizeof(GLuint)));
        else
            glDrawArrays(GL_TRIANGLES, m_frame[frame_index].first, m_frame[frame_index].count);
    }
    glBindVertexArray(0);
}
