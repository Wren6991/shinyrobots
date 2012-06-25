#ifndef WORLD_H_INCLUDED
#define WORLD_H_INCLUDED

#include <btBulletDynamicsCommon.h>
#include "phys_obj.h"

struct world
{
    std::vector<physObj*> objects;

    btBroadphaseInterface *broadphase;
    btDefaultCollisionConfiguration *collisionConfiguration;
    btCollisionDispatcher *dispatcher;
    btSequentialImpulseConstraintSolver *solver;

    public:
    btDiscreteDynamicsWorld *btWorld;

    void addObject(physObj*);
    void render();
    world();
    ~world();
};

//objects are destroyed in world destructor - no need to destroy them elsewhere.


#endif // WORLD_H_INCLUDED
