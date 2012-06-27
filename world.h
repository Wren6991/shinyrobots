#ifndef WORLD_H_INCLUDED
#define WORLD_H_INCLUDED

#include <btBulletDynamicsCommon.h>
#include <map>
#include <vector>
#include "phys_obj.h"

struct tag
{
    typedef enum e_type
    {
        body = 0,
        constraint
    }   e_type;
    e_type type;
    int index;
    tag() {}
    tag(e_type type_, int index_) {type = type_; index = index_;}
};

struct world
{
    std::vector<physObj*> objects;
    std::vector<btTypedConstraint*> constraints;
    std::map<std::string, tag> tags;

    btBroadphaseInterface *broadphase;
    btDefaultCollisionConfiguration *collisionConfiguration;
    btCollisionDispatcher *dispatcher;
    btSequentialImpulseConstraintSolver *solver;

    public:
    btDiscreteDynamicsWorld *btWorld;

    void addObject(physObj*);
    void addConstraint(btTypedConstraint*);
    void render();
    world();
    ~world();
};

//objects are destroyed in world destructor - no need to destroy them elsewhere.


#endif // WORLD_H_INCLUDED
