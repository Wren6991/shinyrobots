#include "util.h"

#include <btBulletDynamicsCommon.h>
#include <iostream>
#include <fstream>
#include <json/json.h>
#include <sstream>
extern "C" {
#include "bsm.h"
}

void strideCopy(char *dest, char *src, int sizeInBytes, int n, int stride_dest, int stride_src)         // you'll have to allocate your own memory before you pass the pointers in!
{
    stride_dest = stride_dest? : sizeInBytes;
    stride_src = stride_src? : sizeInBytes;

    for (int i = 0; i < n; i++)
    {
        for(int byte = 0; byte < sizeInBytes; byte++)
            dest[byte] = src[byte];
        dest += stride_dest;
        src += stride_src;
    }
}

btScalar jsonScalar(Json::Value v, btScalar defaultval)
{
    if (v.isNull())
        return defaultval;
    else
        return v.asDouble();
}

btVector3 jsonVector(Json::Value v, btVector3 defaultvec)
{
    if (!v.isNull())
        return btVector3(v[0.].asDouble(), v[1].asDouble(), v[2].asDouble());
    else
        return defaultvec;
}

btQuaternion jsonQuaternion(Json::Value v, btQuaternion defaultquat)
{
    if (!v.isNull())
        return btQuaternion(v[0.].asDouble(), v[1].asDouble(), v[2].asDouble(), v[3].asDouble());
    else
        return defaultquat;
}

Json::Value jsonParseFile(std::string filename)
{
    std::fstream file(filename.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        std::cout << "Error: could not open file " << filename << "\n";
        return Json::Value();
    }

    file.seekg(0, std::ios::end);
    int length = file.tellg();
    file.seekg(0, std::ios::beg);
    char *buffer = new char[length + 1];
    file.read(buffer, length);

    Json::Reader reader;
    Json::Value root;
    bool parsingSuccessful = reader.parse(buffer, root);
    if (parsingSuccessful)
    {
        return root;
    }
    else
    {
        std::cout << "In file " << filename << ": Parsing error(s):\n" << reader.getFormatedErrorMessages();
        return Json::Value();
    }
}

physObj* staticFromJson(Json::Value obj, std::string currentpath, btTransform transform)
{
    model *mdl = 0;
    btBvhTriangleMeshShape *mesh = 0;
    if (!obj["mesh"].isNull())
    {
        std::stringstream ss;
        ss << currentpath << obj["mesh"].asString();
        mdl = new model(ss.str());
        mesh = new btBvhTriangleMeshShape(collisionMeshFromFile(ss.str()), true);
    }

    return new physObj(0, jsonVector(obj["position"]), mesh, mdl, jsonQuaternion(obj["orientation"]));
}

physObj* dynamicFromJson(Json::Value obj, std::string currentpath, btTransform transform)
{
    model *mdl = 0;
    btCollisionShape *shape = 0;
    if (!obj["view"].isNull())
    {
        std::stringstream ss;
        ss << currentpath << obj["view"].asString();
        mdl = new model(ss.str());
    }
    if (!obj["hull"].isNull())
    {
        std::stringstream ss;
        ss << currentpath << obj["hull"].asString();
        shape = convexHullFromFile(ss.str());
    }
    else if (obj["shape"].isObject())
    {
        std::string type = obj["shape"]["type"].asString();
        if (type == "sphere")
        {
            double radius = obj["shape"]["radius"].asDouble();
            shape = new btSphereShape(radius == 0? 1 : radius);
        }
        else if (type == "box")
        {
            Json::Value extent = obj["shape"]["extent"];
            if (extent.isArray())
                shape = new btBoxShape(btVector3(extent[0.].asDouble(), extent[1].asDouble(), extent[2].asDouble()));
            else
                shape = new btBoxShape(btVector3(1, 1, 1));
        }
    }

    return new physObj(obj["mass"].asDouble(), transform * jsonVector(obj["position"]), shape, mdl, transform * jsonQuaternion(obj["orientation"]), jsonScalar(obj["friction"], 0.5f));
}

btTypedConstraint* constraintFromJson(Json::Value obj, world *gWorld)
{
    if (!obj["tag"].isNull())
        (*gWorld->tags)[obj["tag"].asString()] = tag(tag::constraint, gWorld->constraints.size());
    if (obj["type"].asString() == "axis")
    {
        if (obj["a"].isNull() || obj["b"].isNull())
            return 0;
        std::cout << obj["a"].asString() << ": " << (*gWorld->tags)[obj["a"].asString()].index << "\n";
        std::cout << obj["b"].asString() << ": " << (*gWorld->tags)[obj["b"].asString()].index << "\n";
        btRigidBody *rbA = gWorld->objects[(*gWorld->tags)[obj["a"].asString()].index]->body;
        btRigidBody *rbB = gWorld->objects[(*gWorld->tags)[obj["b"].asString()].index]->body;
        rbA->setSleepingThresholds(0, 0);
        rbB->setSleepingThresholds(0, 0);
        btTransform transA, transB;
        rbA->getMotionState()->getWorldTransform(transA);
        rbB->getMotionState()->getWorldTransform(transB);

        btVector3 axisa, axisb;
        if (!obj["axis"].isNull())
        {
            axisa = jsonVector(obj["axis"], btVector3(1, 0, 0));
            axisb = axisa;
        }
        else
        {
            axisa = jsonVector(obj["axisa"], btVector3(1, 0, 0));
            axisb = jsonVector(obj["axisb"], btVector3(1, 0, 0));
        }
        return new btHingeConstraint(*rbA, *rbB, jsonVector(obj["pivota"]), jsonVector(obj["pivotb"]), axisa, axisb);
    }
}

void loadAssembly(std::string name, btTransform location, std::string path, world *gWorld)
{    if (!gWorld)
    {
        std::cout << "Cannot load assemblies with no world!\n";
        return;
    }
    std::stringstream ss;
    ss << path << "assemblies/" << name << "/";
    std::string assempath = ss.str();
    ss << "info.json";
    Json::Value root = jsonParseFile(ss.str());
    if (root.isNull())
        return;
    gWorld->tags = std::shared_ptr<tag_dict>(new tag_dict(gWorld->tags));
    gWorld->all_tags.push_back(gWorld->tags);
    std::cout << "Loading assembly" << root["name"].asString() << "\n";
    if (root["staticmeshes"].isArray())
    {
        for (unsigned int i = 0; i < root["staticmeshes"].size(); i++)
        {
            physObj *newobj = staticFromJson(root["staticmeshes"][i], assempath);
            btTransform localtrans;
            newobj->body->getMotionState()->getWorldTransform(localtrans);
            newobj->body->getMotionState()->setWorldTransform(location * localtrans);  // translate into world space.
            newobj->body->setMotionState(newobj->body->getMotionState());
            gWorld->addObject(newobj);
            if (!root["staticmeshes"][i]["tag"].isNull())
                (*gWorld->tags)[root["staticmeshes"][i]["tag"].asString()] = tag(tag::body, gWorld->objects.size() - 1);
        }
    }
    if (root["dynamics"].isArray())
    {
        for (unsigned int i = 0; i < root["dynamics"].size(); i++)
        {
            physObj *newobj = dynamicFromJson(root["dynamics"][i], assempath);
            btTransform localtrans;
            newobj->body->getMotionState()->getWorldTransform(localtrans);
            newobj->body->getMotionState()->setWorldTransform(location * localtrans);   // translate into world space.
            newobj->body->setMotionState(newobj->body->getMotionState());               // flush translation to body's m_worldTransform (it's private...)
            gWorld->addObject(newobj);
            if (!root["dynamics"][i]["tag"].isNull())
                (*gWorld->tags)[root["dynamics"][i]["tag"].asString()] = tag(tag::body, gWorld->objects.size() - 1);
        }

    }
    if (root["constraints"].isArray())
    {
        for (unsigned int i = 0; i < root["constraints"].size(); i++)
        {
            gWorld->addConstraint(constraintFromJson(root["constraints"][i], gWorld));

        }
    }
    if (root["assemblies"].isArray())
    {
        for (unsigned int i = 0; i < root["assemblies"].size(); i++)
        {
            Json::Value obj = root["assemblies"][i];
            btVector3 pos = jsonVector(obj["position"]);
            btQuaternion orientation = jsonQuaternion(obj["orientation"]);
            if (!obj["name"].isNull())
                loadAssembly(obj["name"].asString(), btTransform(orientation, pos), path, gWorld);
        }
    }
    gWorld->tags = gWorld->tags->parent;
}

charbuffer getFileContents(std::string filename)
{
    std::fstream file(filename.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Could not open file " << filename << " for reading.\n";
    }
    file.seekg(0, std::ios::end);
    int length = file.tellg();
    file.seekg(0, std::ios::beg);
    char *buffer = new char[length];
    file.read(buffer, length);
    charbuffer result;
    result.buffer = buffer;
    result.length = length;
    return result;
}

btTriangleMesh* collisionMeshFromFile(std::string filename)
{
    charbuffer cb = getFileContents(filename);
    char *buffer = cb.buffer;
    int length = cb.length;

    bsm_header_v1_t *header = new bsm_header_v1_t;
    if (!bsm_read_header_v1((uint8_t*)buffer, length, header))
        std::cerr << "Error: " << filename << " is not a binary static mesh.";
    bsm_position_t *posbuffer = new bsm_position_t[header->num_verts];
    bsm_read_positions((uint8_t*)buffer, length, header, posbuffer);
    bsm_triangle_t *tribuffer = new bsm_triangle_t[header->num_tris];
    bsm_read_tris((uint8_t*)buffer, length, header, tribuffer);

    btTriangleMesh *mesh = new btTriangleMesh();
    for (int i = 0; i < header->num_tris; i++)
    {
        bsm_position_t pos = posbuffer[tribuffer[i].index[0]];
        btVector3 a(pos.x, pos.y, pos.z);                           //btVector3 is actually 4 floats, so if we just did a pointer cast we'd get an access violation for the last pos; so, we instantiate a new btVector3 for each pos.
        pos = posbuffer[tribuffer[i].index[1]];
        btVector3 b(pos.x, pos.y, pos.z);
        pos = posbuffer[tribuffer[i].index[2]];
        btVector3 c(pos.x, pos.y, pos.z);
        mesh->addTriangle(a, b, c);
    }

    std::cout << "Cleaning up:\n";
    delete posbuffer;
    delete tribuffer;
    delete header;
    delete buffer;
    std::cout << "all cleaned up\n";

    return mesh;
}

btConvexHullShape* convexHullFromFile(std::string filename)
{
    std::fstream file(filename.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open())
        std::cerr << "Could not open file " << filename << "for reading.\n";

    file.seekg(0, std::ios::end);
    int length = file.tellg();
    file.seekg(0, std::ios::beg);

    std::cout << length << " bytes\n";

    char *buffer = new char[length + 1];
    file.read(buffer, length);
    bsm_header_v1_t *header = new bsm_header_v1_t;
    if (!bsm_read_header_v1((uint8_t*)buffer, length, header))
        std::cerr << "Error: " << filename << " is not a binary static mesh.";
    bsm_position_t *posbuffer = new bsm_position_t[header->num_verts];
    bsm_read_positions((uint8_t*)buffer, length, header, posbuffer);

    btConvexHullShape *hull = new btConvexHullShape();

    for (int i = 0; i < header->num_verts; i++)
    {
        hull->addPoint(btVector3(posbuffer[i].x, posbuffer[i].y, posbuffer[i].z));
    }


    std::cout << "Cleaning up:\n";
    delete posbuffer;
    delete header;
    delete buffer;
    std::cout << "all cleaned up\n";

    return hull;
}

GLuint makeTexture(GLFWimage img)
{
    GLuint texhandle;
    glGenTextures(1, &texhandle);
    glBindTexture(GL_TEXTURE_2D, texhandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_WRAP_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_WRAP_BORDER);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.Width, img.Height, 0, img.Format, GL_UNSIGNED_BYTE, img.Data);
    return texhandle;
}
/* returns v/u:
 * this is the quaternion that maps u onto v.
 */

btQuaternion vectorQuotient(btVector3 u, btVector3 v)
{
    u.normalize();
    v.normalize();
    return btQuaternion(u.cross(v), acos(u.dot(v)));
}
