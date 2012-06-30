#include "game.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <json/json.h>
#include "lisp/proc.h"

#define RAD_TO_DEGREES (180.0 / 3.14159265358979323846)

game *currentinstance;  // :(

extern std::shared_ptr<environment> global_env;
environment *global_env_ptr;
extern std::shared_ptr<environment> env;

void runLisp(std::string exprs)
{
    parser p(tokenize(exprs));
    try
    {
        while (true)
            proc_eval(p.read());
    }
    catch (exception e)
    {
        if (e.err != "Nothing to read.")
            std::cout << e.err << "\n";
    }
}

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

    if (!captureMouse && mousex > 0 && mousex < width && mousey > 0 && mousey < height && keys.held.MouseL)
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

    if (!keys.held.MouseR)
        gWorld->btWorld->stepSimulation(1/60.f, 10);

    runLisp(
    "(let ((sa 0) (sb 0))"
    " (when (get-key \"I\")"
    "        (setq sa (+ sa 5))"
    "        (setq sb (+ sb 5)))"
    " (when (get-key \"J\")"
    "        (setq sa (+ sa 5))"
    "        (setq sb (- sb 5)))"
    " (when (get-key \"L\")"
    "        (setq sa (- sa 5))"
    "        (setq sb (+ sb 5)))"
    " (when (get-key \"K\")"
    "        (setq sa (- sa 5))"
    "        (setq sb (- sb 5)))"
    " (set-motor\"motora\" sa)"
    " (set-motor \"motorb\" sb))"
    );

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
    // Draw HUD in screen space:
    glPushMatrix();
    glTranslatef(-1.8, -1.8, -2);
    glRotatef(invrotate.getAngle() * RAD_TO_DEGREES, invrotate.getAxis().getX(), invrotate.getAxis().getY(), invrotate.getAxis().getZ());
    glBegin(GL_LINES);
    glColor3f(1.f, 0.f, 0.f);
    glVertex3f(0.f, 0.f, 0.f);
    glVertex3f(0.1f, 0.f, 0.f);
    glColor3f(0.f, 1.f, 0.f);
    glVertex3f(0.f, 0.f, 0.f);
    glVertex3f(0.f, 0.1f, 0.f);
    glColor3f(0.f, 0.f, 1.f);
    glVertex3f(0.f, 0.f, 0.f);
    glVertex3f(0.f, 0.f, 0.1f);
    glColor3f(1.f, 1.f, 1.f);
    glEnd();
    glPopMatrix();

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

btScalar jsonScalar(Json::Value v, btScalar defaultval = 0.f)
{
    if (v.isNull())
        return defaultval;
    else
        return v.asDouble();
}

btVector3 jsonVector(Json::Value v, btVector3 defaultvec = btVector3(0, 0, 0))
{
    if (!v.isNull())
        return btVector3(v[0.].asDouble(), v[1].asDouble(), v[2].asDouble());
    else
        return defaultvec;
}

btQuaternion jsonQuaternion(Json::Value v, btQuaternion defaultquat = btQuaternion(0, 0, 0, 1))
{
    if (!v.isNull())
        return btQuaternion(v[0.].asDouble(), v[1].asDouble(), v[2].asDouble(), v[3].asDouble());
    else
        return defaultquat;
}

void game::loadLevel(std::string level)
{
    if (gWorld)
        delete gWorld;
    gWorld = new world();
    initLisp();
    std::stringstream ss;
    ss << path << "levels/" << level << "/";
    std::string levelpath = ss.str();
    ss << "info.json";
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
            gWorld->addObject(staticFromJson(root["staticmeshes"][i], levelpath));
            if (!root["staticmeshes"][i]["tag"].isNull())
                (*gWorld->tags)[root["staticmeshes"][i]["tag"].asString()] = tag(tag::body, gWorld->objects.size() - 1);
        }

    }
    if (root["dynamics"].isArray())
    {
        for (int i = 0; i < root["dynamics"].size(); i++)
        {
            gWorld->addObject(dynamicFromJson(root["dynamics"][i], levelpath));
            if (!root["dynamics"][i]["tag"].isNull())
                (*gWorld->tags)[root["dynamics"][i]["tag"].asString()] = tag(tag::body, gWorld->objects.size() - 1);
        }

    }
    if (root["constraints"].isArray())
    {
        for (int i = 0; i < root["constraints"].size(); i++)
        {
            gWorld->addConstraint(constraintFromJson(root["constraints"][i]));

        }
    }
    runLisp("(set-motor \"motora\" 10)"
            "(set-motor \"motorb\" (- 0 10))");
}

physObj* game::staticFromJson(Json::Value obj, std::string currentpath)
{
    model *mdl = 0;
    btBvhTriangleMeshShape *mesh = 0;
    if (!obj["mesh"].isNull())
    {
        std::stringstream ss;
        ss << currentpath << obj["mesh"].asString();
        mdl = new model(ss.str())  ;
        mesh = new btBvhTriangleMeshShape(collisionMeshFromFile(ss.str()), true);
    }

    return new physObj(0, jsonVector(obj["position"]), mesh, mdl, jsonQuaternion(obj["orientation"]));
}

physObj* game::dynamicFromJson(Json::Value obj, std::string currentpath)
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

    return new physObj(obj["mass"].asDouble(), jsonVector(obj["position"]), shape, mdl, jsonQuaternion(obj["orientation"]), jsonScalar(obj["friction"], 0.5f));
}

btTypedConstraint* game::constraintFromJson(Json::Value obj)
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

void game::mainloop()
{
    while (running)
    {
        update();
        render();
    }
}

const cell nil(v_symbol, "NIL");

cell proc_get_key(const cell &arglist)
{
    if (!arglist.car)
        return nil;
    cell result = proc_eval(*arglist.car);
    if (result.type == v_number)
        return glfwGetKey(result.n) ? cell(v_symbol, "TRUE") : nil;
    if (result.type == v_string || result.type == v_symbol)
        return glfwGetKey(*result.str.c_str()) ? cell(v_symbol, "TRUE") : nil;
    return nil;
}

cell proc_set_motor(const cell &arglist)
{
    if (!arglist.car || arglist.car->type != v_string)
        throw(exception("Error: expected string as argument to set-motor"));
    if (!arglist.cdr || !arglist.cdr->car)
        throw(exception("Error: expected number as second argument to set-motor"));
    currentinstance->setMotor(arglist.car->str, proc_eval(*arglist.cdr->car).n);
    return cell();
}

void game::setMotor(std::string name, double speed)
{
    std::cout << "setting motor " << name << " to " << speed << "\n";
    tag t = (*gWorld->tags)[name];
    if (t.type != tag::constraint)
    {
        std::cout << "constraint " << name << " does not exist.\n";
        return;
    }

    ((btHingeConstraint*)gWorld->constraints[t.index])->enableAngularMotor(true, speed, 10);
}

void game::initLisp()
{
    global_env_ptr = new environment();
    global_env = std::shared_ptr<environment>(global_env_ptr);
    global_env->vars["PRINT"] = proc_print;
    global_env->vars["WRITE"] = proc_write;
    global_env->vars["EVAL"] = proc_eval_arglist;   //arglist interface to actual eval function.
    global_env->vars["+"] = proc_add;
    global_env->vars["-"] = proc_subtract;
    global_env->vars["*"] = proc_multiply;
    global_env->vars["/"] = proc_divide;
    global_env->vars["="] = proc_equal;
    global_env->vars["<"] = proc_less;
    global_env->vars[">"] = proc_greater;
    global_env->vars["<="] = proc_less_equal;
    global_env->vars[">="] = proc_greater_equal;
    global_env->vars["AND"] = proc_and;
    global_env->vars["OR"] = proc_or;
    global_env->vars["NOT"] = proc_not;
    global_env->vars["IF"] = proc_if;
    global_env->vars["BEGIN"] = proc_begin;
    global_env->vars["DEFINE"] = proc_define;
    global_env->vars["QUOTE"] = proc_quote;
    global_env->vars["QUASI-QUOTE"] = proc_quasi_quote;
    global_env->vars["LAMBDA"] = proc_lambda;
    global_env->vars["MACRO"] = proc_macro;
    global_env->vars["MACROEXPAND-1"] = proc_macroexpand;
    global_env->vars["LISTVARS"] = proc_listvars;
    global_env->vars["TAGBODY"] = proc_tagbody;
    global_env->vars["GO"] = proc_go;
    global_env->vars["CONS"] = proc_cons;
    global_env->vars["CAR"] = proc_car;
    global_env->vars["CDR"] = proc_cdr;
    global_env->vars["LIST"] = proc_list;
    global_env->vars["SETQ"] = proc_setq;
    global_env->vars["NREVERSE"] = proc_nreverse;
    global_env->vars["LET"] = proc_let;
    global_env->vars["SET-MOTOR"] = proc_set_motor;
    global_env->vars["GET-KEY"] = proc_get_key;
    global_env->vars["NIL"] = cell(v_symbol, "NIL");
    global_env->vars["TRUE"] = cell(v_symbol, "TRUE");
    env = global_env;

    runLisp(
    "(define defmacro (macro (name vars &rest body) `(define ,name (macro ,vars ,@body))))"
    "(defmacro defun (name vars &rest body) `(define ,name (lambda ,vars ,@body)))"
    "(defmacro while (expr &rest body) `(tagbody top (if ,expr (begin ,@body (go top))) end))"
    "(defmacro when (cond &rest body) `(if ,cond (begin ,@body)))"
    "(defmacro unless (cond &rest body) `(if (not ,cond) (begin ,@body)))"
    "(defmacro mapcar (func list) `(let ((acc '()) (lis ,list) (fun ,func))"
    " (tagbody top"
    "  (when (cdr lis)"
    "        (push acc (fun (car lis)))"
    "        (setq lis (cdr lis))"
    "        (go top))"
    "  (nreverse acc))))"
    "(defmacro push (list arg) `(setq ,list (cons ,arg ,list)))"
    "(print \"Hello from lisp!\")"
    );
}
