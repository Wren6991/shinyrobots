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
}

void world::addObject(physObj* object)
{
    objects.push_back(object);
    btRigidBody *body = object->body;
    btWorld->getBroadphase();
    btWorld->addRigidBody(body);
}

void world::render()
{
    for (std::vector<physObj*>::iterator iter = objects.begin(); iter != objects.end(); ++iter)
        (*iter)->render();
}
