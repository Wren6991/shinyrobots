#include "app.h"
#include <iostream>

int main(int argc, char **argv)
{
    std::string path = argv[0];
    int slashpos = path.rfind("\\");
    if (slashpos < 0)
        slashpos = path.rfind("/");
    path = path.substr(0, slashpos + 1);
    std::cout << path << "\n";

    app robotsApplication(path);
    robotsApplication.mainLoop();
}
