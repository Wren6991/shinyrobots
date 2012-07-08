#ifndef WORKSHOP_H_INCLUDED
#define WORKSHOP_H_INCLUDED

#include <GL/glew.h>
#include <GL/glfw.h>
#include <string>
#include <vector>
#include "scene.h"
#include "world.h"

class workshopScene: public scene
{
    std::vector<GLuint> thumbnails;
    std::vector<std::string> partnames;
    GLuint cursor;
    GLuint ninepatch;
    int cursorx, cursory;
    bool mouseWasCaptured;
    world *gWorld;
    int selectedItem;
    btCollisionWorld::ClosestRayResultCallback mouseRayCallback;
    btVector3 mouseRayDir;
    btGeneric6DofConstraint *mouseConstraint;
    btScalar mousePerpDist;
    btRigidBody *mouseHeldBody;

    public: struct camera
    {
        btVector3 position;
        btQuaternion orientation;
        float pitch, yaw;
        btVector3 forward, right, up;
        void orientationFromAngles();
    }   camera;

    public:
    void render(sceneInfo &info);
    void update(sceneInfo &info);
    workshopScene(std::string path_);
    ~workshopScene();
};


#endif // WORKSHOP_H_INCLUDED
