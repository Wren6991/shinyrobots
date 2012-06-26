#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <btBulletDynamicsCommon.h>
#include <string>

struct charbuffer
{
    char *buffer;
    int length;
};       //just so we don't have to mess about with const_cast with the bsm lib.

void strideCopy(char *dest, char *src, int sizeInBytes, int n, int stride_dest = 0, int stride_src = 0);         // you'll have to allocate your own memory before you pass the pointers in!
charbuffer getFileContents();
btTriangleMesh* collisionMeshFromFile(std::string filename);
btConvexHullShape* convexHullFromFile(std::string filename);



#endif // UTIL_H_INCLUDED
