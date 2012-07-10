#ifndef APP_H_INCLUDED
#define APP_H_INCLUDED

#include <string>
#include <GL/glew.h>
#include <GL/glfw.h>
#include "scene.h"

class app
{
    sceneInfo info;

    scene *current;

    std::string path;


    static void GLFWCALL dummy_keyCallback(int character, int action);
    static void GLFWCALL charCallback(int character, int action);
    void checkControls();


    public:

    app(std::string path_ = "./");
    void mainLoop();
    void update();
    void render();

    ~app();
};




#endif // APP_H_INCLUDED
