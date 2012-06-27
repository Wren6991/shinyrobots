#include "world.h"

world::world()
{
    broadphase = new btDbvtBroadphase();
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    solver = new btSequentialImpulseConstraintSolver();

    btWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
    btWorld->setGravity(btVector3(0, -10, 0));
}

world::~world()
{
    delete btWorld;
    delete solver;
    delete dispatcher;
    delete collisionConfiguration;
    delete broadphase;
    for (std::vector<physObj*>::iterator iter = objects.begin(); iter != objects.end(); ++iter)
    {
        delete *iter;
    }
    for (std::vector<btTypedConstraint*>::iterator iter = constraints.begin(); iter != constraints.end(); ++iter)
    {
        delete *iter;
    }
}

void world::addObject(physObj *object)
{
    objects.push_back(object);
    btWorld->addRigidBody(object->body);
}

void world::addConstraint(btTypedConstraint *constraint)
{
    constraints.push_back(constraint);
    btWorld->addConstraint(constraint, false);
}

void world::render()
{
    for (std::vector<physObj*>::iterator iter = objects.begin(); iter != objects.end(); ++iter)
        (*iter)->render();
}