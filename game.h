#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include <GL/glew.h>
//#define GLFW_DLL
#include <GL/glfw.h>
#include "phys_obj.h"
#include "world.h"
#include "model.h"
#include "util.h"

class game
{
    int width, height;
    int frame;
    bool running, captureMouse;

    int lastmousex, lastmousey;
    int mousex, mousey;

    world gWorld;

    public: struct camera
    {
        btVector3 position;
        btQuaternion orientation;
        float pitch, yaw;
        btVector3 forward, right, up;
        void orientationFromAngles();
    }   camera;

    struct
    {
        struct
        {
            bool W;
            bool A;
            bool S;
            bool D;
            bool space;
            bool MouseL;
            bool MouseR;
        } held;
        struct
        {
            /*bool W;
            bool A;
            bool S;
            bool D;*/
            bool MouseL;
            bool MouseR;
        } newPress;
    } keys;

    static void GLFWCALL dummy_keyCallback(int character, int action);
    void GLFWCALL keyCallback(int character, int action);
    void checkControls();


    public:

    game();
    void update();
    void render();
    void mainloop();
    ~game();
};



#endif // GAME_H_INCLUDED
