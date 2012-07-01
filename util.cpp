#include "util.h"

#include <iostream>
#include <fstream>
#include <json/json.h>
#include <sstream>
extern "C" {
#include "bsm.h"
}

void strideCopy(char *dest, char *src, int sizeInBytes, int n, int stride_dest, int stride_src)         // you'll have to allocate your own memory before you pass the pointers in!
{
    stride_dest = stride_dest? : sizeInBytes;
    stride_src = stride_src? : sizeInBytes;

    for (int i = 0; i < n; i++)
    {
        for(int byte = 0; byte < sizeInBytes; byte++)
            dest[byte] = src[byte];
        dest += stride_dest;
        src += stride_src;
    }
}

btScalar jsonScalar(Json::Value v, btScalar defaultval)
{
    if (v.isNull())
        return defaultval;
    else
        return v.asDouble();
}

btVector3 jsonVector(Json::Value v, btVector3 defaultvec)
{
    if (!v.isNull())
        return btVector3(v[0.].asDouble(), v[1].asDouble(), v[2].asDouble());
    else
        return defaultvec;
}

btQuaternion jsonQuaternion(Json::Value v, btQuaternion defaultquat)
{
    if (!v.isNull())
        return btQuaternion(v[0.].asDouble(), v[1].asDouble(), v[2].asDouble(), v[3].asDouble());
    else
        return defaultquat;
}

Json::Value jsonParseFile(std::string filename)
{
    std::fstream file(filename.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        std::cout << "Error: could not open file " << filename << "\n";
        return Json::Value();
    }

    file.seekg(0, std::ios::end);
    int length = file.tellg();
    file.seekg(0, std::ios::beg);
    char *buffer = new char[length + 1];
    file.read(buffer, length);

    Json::Reader reader;
    Json::Value root;
    bool parsingSuccessful = reader.parse(buffer, root);
    if (parsingSuccessful)
    {
        return root;
    }
    else
    {
        std::cout << "In file " << filename << ": Parsing error(s):\n" << reader.getFormatedErrorMessages();
        return Json::Value();
    }
}

charbuffer getFileContents(std::string filename)
{
    std::fstream file(filename.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Could not open file " << filename << " for reading.\n";
    }
    file.seekg(0, std::ios::end);
    int length = file.tellg();
    file.seekg(0, std::ios::beg);
    char *buffer = new char[length];
    file.read(buffer, length);
    charbuffer result;
    result.buffer = buffer;
    result.length = length;
    return result;
}

btTriangleMesh* collisionMeshFromFile(std::string filename)
{
    charbuffer cb = getFileContents(filename);
    char *buffer = cb.buffer;
    int length = cb.length;

    bsm_header_v1_t *header = new bsm_header_v1_t;
    if (!bsm_read_header_v1((uint8_t*)buffer, length, header))
        std::cerr << "Error: " << filename << " is not a binary static mesh.";
    bsm_position_t *posbuffer = new bsm_position_t[header->num_verts];
    bsm_read_positions((uint8_t*)buffer, length, header, posbuffer);
    bsm_triangle_t *tribuffer = new bsm_triangle_t[header->num_tris];
    bsm_read_tris((uint8_t*)buffer, length, header, tribuffer);

    btTriangleMesh *mesh = new btTriangleMesh();
    for (int i = 0; i < header->num_tris; i++)
    {
        bsm_position_t pos = posbuffer[tribuffer[i].index[0]];
        btVector3 a(pos.x, pos.y, pos.z);                           //btVector3 is actually 4 floats, so if we just did a pointer cast we'd get an access violation for the last pos; so, we instantiate a new btVector3 for each pos.
        pos = posbuffer[tribuffer[i].index[1]];
        btVector3 b(pos.x, pos.y, pos.z);
        pos = posbuffer[tribuffer[i].index[2]];
        btVector3 c(pos.x, pos.y, pos.z);
        mesh->addTriangle(a, b, c);
    }

    std::cout << "Cleaning up:\n";
    delete posbuffer;
    delete tribuffer;
    delete header;
    delete buffer;
    std::cout << "all cleaned up\n";

    return mesh;
}

btConvexHullShape* convexHullFromFile(std::string filename)
{
    std::fstream file(filename.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open())
        std::cerr << "Could not open file " << filename << "for reading.\n";

    file.seekg(0, std::ios::end);
    int length = file.tellg();
    file.seekg(0, std::ios::beg);

    std::cout << length << " bytes\n";

    char *buffer = new char[length + 1];
    file.read(buffer, length);
    bsm_header_v1_t *header = new bsm_header_v1_t;
    if (!bsm_read_header_v1((uint8_t*)buffer, length, header))
        std::cerr << "Error: " << filename << " is not a binary static mesh.";
    bsm_position_t *posbuffer = new bsm_position_t[header->num_verts];
    bsm_read_positions((uint8_t*)buffer, length, header, posbuffer);

    btConvexHullShape *hull = new btConvexHullShape();

    for (int i = 0; i < header->num_verts; i++)
    {
        hull->addPoint(btVector3(posbuffer[i].x, posbuffer[i].y, posbuffer[i].z));
    }


    std::cout << "Cleaning up:\n";
    delete posbuffer;
    delete header;
    delete buffer;
    std::cout << "all cleaned up\n";

    return hull;
}
