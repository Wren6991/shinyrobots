#ifndef WORKSHOP_H_INCLUDED
#define WORKSHOP_H_INCLUDED

#include <GL/glew.h>
#include <GL/glfw.h>
#include <string>
#include <vector>
#include "scene.h"
#include "world.h"

struct ninePatch
{
    GLuint texture;
    int width, height;
    int   soffsleft, soffsright, soffsbottom, soffstop;     //screen-space offsets (pixels)
    float toffsleft, toffsright, toffsbottom, toffstop;     //texture offsets of patch divisions (0.f->1.f)

    ninePatch(GLuint texture_ = 0, int soffsleft_ = 8, int soffsright_ = 8, int soffsbottom_ = 8, int soffstop_ = 8, float toffsleft_ = 0.25, float toffsright_ = 0.75, float toffsbottom_ = 0.25, float toffstop_ = 0.75);
    void draw(float left, float right, float bottom, float top);
};

class workshopScene: public scene
{
    std::vector<GLuint> thumbnails;
    std::vector<std::string> partnames;
    std::vector<GLuint> tooltextures;
    GLuint cursor;
    ninePatch panel;
    ninePatch button;
    ninePatch bubble;
    GLuint font;
    GLuint stbfont;
    int cursorx, cursory;
    float mousevelx, mousevely;
    bool mouseWasCaptured;
    world *gWorld;
    int selectedItem;
    int selectedTool;
    int toolListOffset;
    btCollisionWorld::ClosestRayResultCallback mouseRayCallback;
    btVector3 mouseRayDir;
    btGeneric6DofConstraint *mouseConstraint;
    btScalar mousePerpDist;
    btRigidBody *mouseHeldBody;
    btCollisionWorld::ClosestRayResultCallback axisResult;
    btVector3 axisFirstPivot;
    btVector3 axisFirstNormal;
    bool axisHasFirst;

    GLuint textureFromFile(std::string name, std::string reldir = "data/");
    void renderChar(int x, int y, char c);
    void print(int x, int y, std::string str);
    void stbprint(int x, int y, char *str);
    void showBubble(int x, int y, std::string str);

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
