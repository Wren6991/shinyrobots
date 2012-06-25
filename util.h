#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <btBulletDynamicsCommon.h>

void strideCopy(char *dest, char *src, int sizeInBytes, int n, int stride_dest = 0, int stride_src = 0);         // you'll have to allocate your own memory before you pass the pointers in!
btTriangleMesh *collisionMeshFromFile(std::string filename);


#endif // UTIL_H_INCLUDED
