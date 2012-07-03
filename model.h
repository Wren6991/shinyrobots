#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED

#include <string>
#include <GL/glew.h>

struct vertex
{
    //Position:
    float x, y, z;
    //Normal:
    float nx, ny, nz;
    //Texcoord:
    float u, v;
};

class model
{
    GLuint vertexbuffer;
    GLuint indexbuffer;
    int ntriangles;
    public:
    void render();
    model(std::string filename);
    ~model();
};

#endif // MODEL_H_INCLUDED
