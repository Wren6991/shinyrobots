#include "game.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <json/json.h>
#include "lisp/proc.h"
#include "util.h"

#define RAD_TO_DEGREES (180.0 / 3.14159265358979323846)

extern std::shared_ptr<environment> global_env;
environment *global_env_ptr;
extern std::shared_ptr<environment> env;

gameScene *currentinstance;

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

void gameScene::camera::orientationFromAngles()
{
    orientation.setEuler(yaw, pitch, 0);
    btQuaternion qforward = orientation * btQuaternion(0, 0, -1, 0) * orientation.inverse();
    forward = btVector3(qforward.getX(), qforward.getY(), qforward.getZ());
    btQuaternion qright = orientation * btQuaternion(1, 0, 0, 0) * orientation.inverse();
    right = btVector3(qright.getX(), qright.getY(), qright.getZ());
    btQuaternion qup = orientation * btQuaternion(0, 1, 0, 0) * orientation.inverse();
    up = btVector3(qup.getX(), qup.getY(), qup.getZ());
}

gameScene::gameScene(std::string path_)
{
    currentinstance = this;
    gWorld = 0;
    path = path_;
    /*for(int i = 0; i < sizeof(keys); i++)
        ((char*)&keys)[i] = 0;
    currentinstance = this;         //so the key callback can access the key state... :(*/
    camera.position = btVector3(0, 0, 10);
    camera.pitch = 0;
    camera.yaw = 0;
    camera.orientationFromAngles();
    //glfwSetKeyCallback(dummy_keyCallback);

    loadLevel("arena");
}


void gameScene::update(sceneInfo &info)
{
    camera.pitch -= info.dmousey * 0.02;
    camera.yaw -= info.dmousex * 0.02;
    camera.orientationFromAngles();

    sceneInfo::keyState &keys = info.keys;

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
}

void gameScene::render(sceneInfo &info)
{
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glFrustum((-1.f * info.width) / info.height, (1.f * info.width) / info.height, -1.f, 1.f, 1.f, 1000.f);

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

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
}

gameScene::~gameScene()
{
    if (gWorld)
        delete gWorld;
}

void gameScene::loadLevel(std::string level)
{
    if (gWorld)
        delete gWorld;
    gWorld = new world();
    initLisp();
    std::stringstream ss;
    ss << path << "levels/" << level << "/";
    std::string levelpath = ss.str();
    ss << "info.json";
    Json::Value root = jsonParseFile(ss.str());
    if (root.isNull())
        return;
    std::cout << root["name"].asString() << "\n" << root["description"].asString() << "\n";
    if (root["staticmeshes"].isArray())
    {
        for (unsigned int i = 0; i < root["staticmeshes"].size(); i++)
        {
            gWorld->addObject(staticFromJson(root["staticmeshes"][i], levelpath));
            if (!root["staticmeshes"][i]["tag"].isNull())
                (*gWorld->tags)[root["staticmeshes"][i]["tag"].asString()] = tag(tag::body, gWorld->objects.size() - 1);
        }

    }
    if (root["dynamics"].isArray())
    {
        for (unsigned int i = 0; i < root["dynamics"].size(); i++)
        {
            gWorld->addObject(dynamicFromJson(root["dynamics"][i], levelpath));
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

void gameScene::setMotor(std::string name, double speed)
{
    tag t = (*gWorld->tags)[name];
    if (t.type != tag::constraint)
    {
        std::cout << "constraint " << name << " does not exist.\n";
        return;
    }

    ((btHingeConstraint*)gWorld->constraints[t.index])->enableAngularMotor(true, speed, 10);
}

void gameScene::initLisp()
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
