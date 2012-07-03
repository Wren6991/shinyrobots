#ifndef WORKSHOP_H_INCLUDED
#define WORKSHOP_H_INCLUDED

#include <GL/glew.h>
#include <GL/glfw.h>
#include <string>
#include <vector>
#include "scene.h"

class workshopScene: public scene
{
    std::vector<GLuint> thumbnails;
    std::vector<std::string> partnames;
    GLuint cursor;
    int cursorx, cursory;
    bool mouseWasCaptured;
    public:
    void render(sceneInfo &info);
    void update(sceneInfo &info);
    workshopScene(std::string path_);
};


#endif // WORKSHOP_H_INCLUDED
