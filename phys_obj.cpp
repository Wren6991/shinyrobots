#include "phys_obj.h"
#include <iostream>

#define RAD_TO_DEGREES (180.0 / 3.14159265358979323846)

void physObj::render()
{
    glPushMatrix();
    btTransform trans;
    body->getMotionState()->getWorldTransform(trans);
    glTranslatef(trans.getOrigin().getX(), trans.getOrigin().getY(), trans.getOrigin().getZ());
    glRotatef(trans.getRotation().getAngle() * RAD_TO_DEGREES, trans.getRotation().getAxis().getX(), trans.getRotation().getAxis().getY(), trans.getRotation().getAxis().getZ());

    if (mdl)
        mdl->render();
    else
    {
        glBegin(GL_TRIANGLES);
        glColor3f(0.1f, 0.0f, 0.0f );
        glVertex3f(0.0f, 3.0f, -4.0f);
        glColor3f(0.0f, 1.0f, 0.0f );
        glVertex3f(3.0f, -2.0f, -4.0f);
        glColor3f(0.0f, 0.0f, 1.0f );
        glVertex3f(-3.0f, -2.0f, -4.0f);
        glEnd();
        glBegin(GL_TRIANGLES);
        glColor3f(0.0f, 0.1f, 0.0f );
        glVertex3f(0.0f, 3.0f, -3.0f);
        glColor3f(0.0f, 0.0f, 1.0f );
        glVertex3f(3.0f, -2.0f, -2.0f);
        glColor3f(1.0f, 0.0f, 0.0f );
        glVertex3f(-3.0f, -2.0f, 2.0f);
        glEnd();
    }
    glPopMatrix();
}

physObj::physObj(btScalar mass = 0, btVector3 pos = btVector3(0, 0, 0), btCollisionShape *shape, model *mdl_)
{
    mdl = mdl_;
    if (!shape)
        shape = new btBoxShape(btVector3(1, 1, 1));
    std::cout << "Instantiated new physObj\n";
    motionstate = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), pos));
    btVector3 inertia(0, 0, 0);
    shape->calculateLocalInertia(mass, inertia);
    btRigidBody::btRigidBodyConstructionInfo coninfo(mass, motionstate, shape, inertia);
    body = new btRigidBody(coninfo);
}
