#ifndef PHYS_OBJ_H_INCLUDED
#define PHYS_OBJ_H_INCLUDED

#include <GL/glew.h>
#include <btBulletDynamicsCommon.h>
#include <string>
#include <vector>

#include "model.h"

struct physObj
{
    btCollisionShape* shape;
    btDefaultMotionState *motionstate;
    btRigidBody *body;
    model *mdl;
    std::string tag;

    void render();

    physObj(btScalar mass, btVector3 pos, btCollisionShape *shape_ = new btBoxShape(btVector3(1, 1, 1)), model *mdl_ = 0, btQuaternion orientation = btQuaternion(0, 0, 0, 1), btScalar friction = 0.5f, std::string tag_ = "");
};

#endif // PHYS_OBJ_H_INCLUDED
