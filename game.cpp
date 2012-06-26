#include "game.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <json/json.h>

#define RAD_TO_DEGREES (180.0 / 3.14159265358979323846)

game *currentinstance;  // :(

void game::camera::orientationFromAngles()
{
    orientation.setEuler(yaw, pitch, 0);
    btQuaternion qforward = orientation * btQuaternion(0, 0, -1, 0) * orientation.inverse();
    forward = btVector3(qforward.getX(), qforward.getY(), qforward.getZ());
    btQuaternion qright = orientation * btQuaternion(1, 0, 0, 0) * orientation.inverse();
    right = btVector3(qright.getX(), qright.getY(), qright.getZ());
    btQuaternion qup = orientation * btQuaternion(0, 1, 0, 0) * orientation.inverse();
    up = btVector3(qup.getX(), qup.getY(), qup.getZ());
}

game::game(std::string path_)
{
    gWorld = 0;
    path = path_;
    for(int i = 0; i < sizeof(keys); i++)
        ((char*)&keys)[i] = 0;
    currentinstance = this;         //so the key callback can access the key state... :(
    camera.position = btVector3(0, 0, 10);
    camera.pitch = 0;
    camera.yaw = 0;
    camera.orientationFromAngles();

    glfwInit();
    if( !glfwOpenWindow(1024, 768, 0, 0, 0, 0, 0, 0, GLFW_WINDOW))
    {
        glfwTerminate();
    }
    glfwSetWindowTitle("LISP");
    glfwSwapInterval(1);
    glfwSetKeyCallback(dummy_keyCallback);

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        std::cout << "GLEW init failed: \"" << glewGetErrorString(err) << "\". Shutting down...\n";
        char c;
        std::cin >> c;
    }
    else
    {
        std::cout << "GLEW OK! OGL version: " << GLEW_VERSION_MAJOR << "." << GLEW_VERSION_MINOR << "\n";
    }

    running = true;
    captureMouse = false;

    /*gWorld->addObject(new physObj(1, btVector3(0, 0, 0), 0, new model("C:/Users/Owner/Documents/cube2.bsm")));
    gWorld->addObject(new physObj(1, btVector3(1, 5, 0), convexHullFromFile("C:/Users/Owner/Documents/bob.bsm"), new model("C:/Users/Owner/Documents/bob.bsm")));
    btBvhTriangleMeshShape *msh = new btBvhTriangleMeshShape(collisionMeshFromFile("C:/Users/Owner/Documents/arena_yz.bsm"), true);
    gWorld->addObject(new physObj(0, btVector3(0, -5, 0), msh, new model("C:/Users/Owner/Documents/arena_yz.bsm")));
    gWorld->objects[0]->body->applyForce(btVector3(0, 200, 0), btVector3(1, 1, 0.8));*/
    loadLevel("arena");



}

void GLFWCALL game::dummy_keyCallback(int character, int action)         //we can't pass a non-static member function as a callback, but a static function can't access the data - this should do for now.
{
    switch(character)
    {
        case 'W':
            currentinstance->keys.held.W = action;
            break;
        case 'S':
            currentinstance->keys.held.S = action;
            break;
        case 'A':
            currentinstance->keys.held.A = action;
            break;
        case 'D':
            currentinstance->keys.held.D = action;
            break;
        case ' ':
            currentinstance->keys.held.space = action;
        default:
            break;
    }
}

void GLFWCALL game::keyCallback(int character, int action)
{

}

void game::checkControls()
{
    if(glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT))
    {
            keys.newPress.MouseL = !keys.held.MouseL;
            keys.held.MouseL = true;
    }
    else
    {
        keys.newPress.MouseL = false;
        keys.held.MouseL = false;
    }

    if(glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT))
    {
            keys.newPress.MouseR = !keys.held.MouseR;
            keys.held.MouseR = true;
    }
    else
    {
        keys.newPress.MouseR = false;
        keys.held.MouseR = false;
    }
}



void game::update()
{
    checkControls();
    lastmousex = mousex;
    lastmousey = mousey;
    glfwGetMousePos(&mousex, &mousey);

    if (mousex > 0 && mousex < width && mousey > 0 && mousey < height && keys.held.MouseL)
    {
         captureMouse = true;
         glfwSetMousePos(width / 2, height / 2);
         mousex = width / 2;
         mousey = height / 2;
    }


    if (!glfwGetKey(GLFW_KEY_ESC) && glfwGetWindowParam(GLFW_ACTIVE) && captureMouse)
    {
        glfwDisable(GLFW_MOUSE_CURSOR);
        camera.pitch -= (mousey - height / 2) * 0.01;
        camera.yaw -= (mousex - width / 2) * 0.01;
        glfwSetMousePos(width / 2, height / 2);

    }
    else
    {
        glfwEnable(GLFW_MOUSE_CURSOR);
        captureMouse = false;
    }

    camera.orientationFromAngles();

    if (keys.held.W)
    {
        camera.position += camera.forward * 0.1;
    }
    else if (keys.held.S)
    {
        camera.position -= camera.forward * 0.1;
    }
    if (keys.held.D)
    {
        camera.position += camera.right * 0.1;
    }
    else if (keys.held.A)
    {
        camera.position -= camera.right * 0.1;
    }

    if (keys.held.space)
    {
        gWorld->addObject(new physObj(1, camera.position, 0, new model("C:/Users/Owner/Documents/cube.bsm")));
    }

    gWorld->btWorld->stepSimulation(1/60.f, 10);

    running = glfwGetWindowParam(GLFW_OPENED);
}

void game::render()
{
    glfwGetWindowSize( &width, &height );
    height = height > 0 ? height : 1;

    glViewport( 0, 0, width, height );

    glClearColor( 0.5f, 0.5f, 0.5f, 0.0f );
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glFrustum((-1.f * width) / height, (1.f * width) / height, -1.f, 1.f, 1.f, 1000.f);

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    camera.orientation.setEuler(camera.yaw, camera.pitch, 0);
    btQuaternion invrotate = camera.orientation.inverse();
    glRotatef(invrotate.getAngle() * RAD_TO_DEGREES, invrotate.getAxis().getX(), invrotate.getAxis().getY(), invrotate.getAxis().getZ());

    glTranslatef(-camera.position.getX(), -camera.position.getY(), -camera.position.getZ());

    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 50.0 };
    GLfloat light_position[] = { 1.2, 1.1, 1.0, 0.0 };
    glShadeModel (GL_SMOOTH);

    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_COLOR_MATERIAL);

    glEnable(GL_CULL_FACE);

    gWorld->render();
    glfwSwapBuffers();
}

game::~game()
{
    glfwTerminate();
}

void game::loadLevel(std::string level)
{
    if (gWorld)
        delete gWorld;
    gWorld = new world();
    std::stringstream ss;
    ss << path << "levels/" << level << "/info.json";
    std::string infopath = ss.str();
    std::fstream file(infopath.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        std::cout << "Error: could not open file " << infopath << "\n";
        return;
    }
    file.seekg(0, std::ios::end);
    int length = file.tellg();
    file.seekg(0, std::ios::beg);
    char *buffer = new char[length + 1];
    file.read(buffer, length);

    Json::Reader reader;
    Json::Value root;
    bool parsingSuccessful = reader.parse(buffer, root);
    if (!parsingSuccessful)
    {
        std::cout << "Parsing error(s):\n" << reader.getFormatedErrorMessages();
        return;
    }
    std::cout << root["name"].asString() << "\n" << root["description"].asString() << "\n";
    if (root["staticmeshes"].isArray())
    {
        for (int i = 0; i < root["staticmeshes"].size(); i++)
        {
            Json::Value obj = root["staticmeshes"][i];
            model *mdl = 0;
            btBvhTriangleMeshShape *mesh = 0;
            if (!obj["mesh"].isNull())
            {
                std::stringstream ss;
                ss << path << "levels/" << level << "/" << obj["mesh"].asString();
                mdl = new model(ss.str())  ;
                mesh = new btBvhTriangleMeshShape(collisionMeshFromFile(ss.str()), true);
            }
            Json::Value loc = obj["location"];
            btVector3 pos(0, 0, 0);
            if (loc.isArray())
                pos = btVector3(loc[0.].asDouble(), loc[1.].asDouble(), loc[2u].asDouble());
            gWorld->addObject(new physObj(0, pos, mesh, mdl));
        }
    }
    if (root["dynamics"].isArray())
    {
        for (int i = 0; i < root["dynamics"].size(); i++)
        {
            Json::Value obj = root["dynamics"][i];
            model *mdl = 0;
            btCollisionShape *shape = 0;
            if (!obj["view"].isNull())
            {
                std::stringstream ss;
                ss << path << "levels/" << level << "/" << obj["view"].asString();
                mdl = new model(ss.str());
            }
            if (!obj["hull"].isNull())
            {
                std::stringstream ss;
                ss << path << "levels/" << level << "/" << obj["hull"].asString();
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
            Json::Value loc = obj["location"];
            btVector3 pos(0, 0, 0);
            if (loc.isArray())
                pos = btVector3(loc[0.].asDouble(), loc[1.].asDouble(), loc[2u].asDouble());
            gWorld->addObject(new physObj(obj["mass"].asDouble(), pos, shape, mdl));
        }
    }
}

void game::mainloop()
{
    while (running)
    {
        update();
        render();
    }
}
