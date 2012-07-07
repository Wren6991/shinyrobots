#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include <GL/glew.h>
#include <GL/glfw.h>
#include <json/json.h>
#include "scene.h"
#include "phys_obj.h"
#include "world.h"
#include "model.h"
#include "util.h"

class gameScene: public scene
{
    world *gWorld;

    public: struct camera
    {
        btVector3 position;
        btQuaternion orientation;
        float pitch, yaw;
        btVector3 forward, right, up;
        void orientationFromAngles();
    }   camera;

    public:

    gameScene(std::string path_ = "./");
    void update(sceneInfo &info);
    void render(sceneInfo &info);
    void loadLevel(std::string level = "test");
    void initLisp();
    void setMotor(std::string name, double speed);

    ~gameScene();
};



#endif // GAME_H_INCLUDED
