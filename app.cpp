#include "app.h"
#include "game.h"
#include "workshop.h"
#include <iostream>

app *currentInstance;

app::app(std::string path_)
{
    currentInstance = this;     // :(
    path = path_;
    for (int i = 0; i < sizeof(sceneInfo); i++)
        ((char*)&info)[i] = 0;
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

    info.currentscene = new workshopScene(path);

    info.running = true;
    info.captureMouse = false;
}

void GLFWCALL app::dummy_keyCallback(int character, int action)         //we can't pass a non-static member function as a callback, but a static function can't access the data - this should do for now.
{
    sceneInfo::keyState &keys = currentInstance->info.keys;
    switch(character)
    {
        case 'W':
            keys.held.W = action;
            break;
        case 'S':
            keys.held.S = action;
            break;
        case 'A':
            keys.held.A = action;
            break;
        case 'D':
            keys.held.D = action;
            break;
        case ' ':
            keys.held.space = action;
        default:
            break;
    }
}

void app::checkControls()
{
    sceneInfo::keyState &keys = currentInstance->info.keys;
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
    keys.dmouseWheel = glfwGetMouseWheel() - keys.mouseWheel;
    keys.mouseWheel += keys.dmouseWheel;
}

void app::mainLoop()
{
    while (info.running)
    {
        update();
        render();
    }
}

void app::update()
{
    checkControls();
    info.lastmousex = info.mousex;
    info.lastmousey = info.mousey;
    glfwGetMousePos(&info.mousex, &info.mousey);

    if (!info.captureMouse && info.mousex > 0 && info.mousex < info.width && info.mousey > 0 && info.mousey < info.height && info.keys.held.MouseL)
    {
         info.captureMouse = true;
         glfwSetMousePos(info.width / 2, info.height / 2);
         info.mousex = info.width / 2;
         info.mousey = info.height / 2;
    }


    if (!glfwGetKey(GLFW_KEY_ESC) && glfwGetWindowParam(GLFW_ACTIVE) && info.captureMouse)
    {
        glfwDisable(GLFW_MOUSE_CURSOR);
        info.dmousey = info.mousey - info.height / 2;
        info.dmousex = info.mousex - info.width / 2;
        glfwSetMousePos(info.width / 2, info.height / 2);
    }
    else
    {
        glfwEnable(GLFW_MOUSE_CURSOR);
        info.captureMouse = false;
    }

    info.currentscene->update(info);

    info.running = glfwGetWindowParam(GLFW_OPENED);
}

void app::render()
{
    glfwGetWindowSize( &info.width, &info.height );
    info.height = info.height > 0 ? info.height : 1;

    glViewport(0, 0, info.width, info.height);

    glClearColor( 0.5f, 0.5f, 0.5f, 0.0f );
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    info.currentscene->render(info);

    glfwSwapBuffers();
}

app::~app()
{
    glfwTerminate();
}
