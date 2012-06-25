#include <fstream>
#include <assert.h>
#include <iostream>
#include <GL/glew.h>
#include "model.h"
#include "util.h"
//extern "C" {
#include "bsm.h"
//}

#define BUFFER_OFFSET(i) (reinterpret_cast<void*>(i))


void model::render()
{
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glVertexPointer(3, GL_FLOAT, 32, BUFFER_OFFSET(0));
    glNormalPointer(GL_FLOAT, 32, BUFFER_OFFSET(12));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer);
    glDrawElements(GL_TRIANGLES, ntriangles * 3, GL_UNSIGNED_INT, BUFFER_OFFSET(0));

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}

model::model(std::string filename)
{
    assert(sizeof(vertex) == sizeof(bsm_position_t) + sizeof(bsm_normal_t) + sizeof(bsm_texcoord_t));
    std::fstream file(filename.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open())
        std::cerr << "Cannot open file " << filename << std::endl;
    file.seekg(0, std::ios::end);
    int length = file.tellg();
    file.seekg(0, std::ios::beg);

    std::cout << length << " bytes\n";

    char *buffer = new char[length];
    file.read(buffer, length);

    bsm_header_v1_t *header = new bsm_header_v1_t;
    if (!bsm_read_header_v1((uint8_t*)buffer, length, header))
    {
        std::cout << "Error: " << filename << "is not a binary static mesh.";
    }
    std::cout << "Num. tris: " << header->num_tris << "\n";

    bool success = true;

    bsm_position_t *posbuffer = new bsm_position_t[header->num_verts];
    bsm_normal_t *normalbuffer = new bsm_normal_t[header->num_verts];
    bsm_texcoord_t *texcoordbuffer = new bsm_texcoord_t[header->num_verts];
    success = success && bsm_read_positions((uint8_t*)buffer, length, header, posbuffer);
    success = success && bsm_read_normals((uint8_t*)buffer, length, header, normalbuffer);
    success = success && bsm_read_texcoords((uint8_t*)buffer, length, header, texcoordbuffer);

    if (success)
        std::cout << "Hells yeah!\n";
    else
        std::cout << "Whoops!\n";


    vertex *vdatabuffer = new vertex[header->num_verts];
    strideCopy((char*)vdatabuffer, (char*)(posbuffer), sizeof(bsm_position_t), header->num_verts, sizeof(vertex));
    strideCopy(((char*)vdatabuffer) + 3 * sizeof(float), (char*)(normalbuffer), sizeof(bsm_normal_t), header->num_verts, sizeof(vertex));
    strideCopy(((char*)vdatabuffer) + 6 * sizeof(float), (char*)(texcoordbuffer), sizeof(bsm_texcoord_t), header->num_verts, sizeof(vertex));

    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, header->num_verts * sizeof(vertex), vdatabuffer, GL_STATIC_DRAW);

    delete vdatabuffer;
    delete posbuffer;
    delete normalbuffer;
    delete texcoordbuffer;

    bsm_triangle_t *triangles = new bsm_triangle_t[header->num_tris];
    bsm_read_tris((uint8_t*)buffer, length, header, triangles);
    glGenBuffers(1, &indexbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, bsm_tris_bytes(header), triangles, GL_STATIC_DRAW);

    ntriangles = header->num_tris;

    delete triangles;
    delete header;
    delete buffer;
}


model::~model()
{
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &indexbuffer);
}
