#include "world.h"

world::world()
{
    broadphase = new btDbvtBroadphase();
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    solver = new btSequentialImpulseConstraintSolver();

    btWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
    btWorld->setGravity(btVector3(0, -10, 0));
    global_tags = std::shared_ptr<tag_dict> (new tag_dict());
    tags = global_tags;
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

void world::removeBody(btRigidBody *body)
{
    std::vector<btTypedConstraint*>::iterator iter;
    for (iter = gWorld->constraints.begin(); iter != gWorld->constraints.end(); iter++)
    {
        if (&((*iter)->getRigidBodyA()) == body || &((*iter)->getRigidBodyB()) == body)
        {
            gWorld->btWorld->removeConstraint(*iter);
            delete *iter;
            gWorld->constraints.erase(iter);
        }
        gWorld->btWorld->removeRigidBody(body);
        for (std::vector<physObj*>::iterator iter = gWorld->objects.begin(); iter != gWorld->objects.end(); iter++)
        {
            if ((*iter)->body == body)
            {
                gWorld->objects.erase(iter);
                break;
            }
        }
    }
}

tag& tag_dict::operator[](std::string name)
{
    std::map<std::string, tag>::iterator iter = tags.find(name);
    if (iter != tags.end())
        return iter->second;
    if (parent)
        return (*parent)[name];
    return tags[name];  // we drop through to this if we're in the global env; if so, we look up an empty tag and return the new reference.
}
