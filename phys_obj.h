#ifndef PHYS_OBJ_H_INCLUDED
#define PHYS_OBJ_H_INCLUDED

#include <GL/gl.h>
#include <btBulletDynamicsCommon.h>
#include <vector>

#include "model.h"

struct physObj
{
    GLuint vbo;
    GLuint ibo;

    btCollisionShape* shape;
    btDefaultMotionState *motionstate;
    btRigidBody *body;
    model *mdl;

    void render();
    void addShape(btCollisionShape*);

    physObj(btScalar mass, btVector3 pos, btCollisionShape *shape_ = new btBoxShape(btVector3(1, 1, 1)), model *mdl_ = 0, btQuaternion orientation = btQuaternion(0, 0, 0, 1));
};

#endif // PHYS_OBJ_H_INCLUDED
