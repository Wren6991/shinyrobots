#ifndef SCENE_H_INCLUDED
#define SCENE_H_INCLUDED
#include <string>

class scene;

struct sceneInfo
{
    int width, height;
    bool running, captureMouse;
    int lastmousex, lastmousey;
    int mousex, mousey;
    int dmousex, dmousey;       //for capture mode.
    struct keyState
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
        int mouseWheel;
        int dmouseWheel;
    } keys;
    scene *currentscene;
};

class scene
{
    public:
    std::string path;
    virtual void update(sceneInfo &info) = 0;
    virtual void render(sceneInfo &info) = 0;
};


#endif // SCENE_H_INCLUDED
