#include "util.h"

#include <iostream>
#include <fstream>
#include "bsm.h"

void strideCopy(char *dest, char *src, int sizeInBytes, int n, int stride_dest = 0, int stride_src = 0)         // you'll have to allocate your own memory before you pass the pointers in!
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

void collisionMeshFromFile(std::string filename)
{

}
