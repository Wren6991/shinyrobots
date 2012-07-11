#include "workshop.h"

#include <dirent.h>
#include <iostream>
#include <sstream>
#include "util.h"
#include "stb_font_consolas_12_usascii.inl"

#define RAD_TO_DEGREES (180.0 / 3.14159265358979323846)

const int UI_SIZE = 96;
const int UI_SMALL = (UI_SIZE * 2) / 3;
static stb_fontchar fontdata[STB_SOMEFONT_NUM_CHARS];

workshopScene::workshopScene(std::string path_)
:mouseRayCallback(btVector3(0, 0, 0), btVector3(0, 0, 0)),
 axisResult(btVector3(0, 0, 0), btVector3(0, 0, 0))
{
    path = path_;
    DIR *dir;
    dirent *ent;
    gWorld = new world();
    gWorld->btWorld->setGravity(btVector3(0, 0, 0));
    mouseConstraint = 0;
    std::stringstream ss;
    ss << path << "data/plane_10x10.bsm";
    gWorld->addObject(new physObj(0, btVector3(0, -1, 0), new btStaticPlaneShape(btVector3(0, 1, 0), 1), new model(ss.str())));
    ss.str("");
    ss << path << "assemblies";
    dir = opendir(ss.str().c_str());
    if (dir == NULL)
    {
        std::cout << "Could not open assemblies directory.\n";
        return;
    }
    int i = 0;
    while ((ent = readdir(dir)) != NULL)
    {
        if (ent->d_type == DT_DIR && ++i > 2)     //discard "." and ".."
        {
            partnames.push_back(ent->d_name);
        }
    }
    closedir(dir);

    for (unsigned int i = 0; i < partnames.size(); i++)
    {
        std::cout << "Part: " << partnames[i] << "\n";
        std::stringstream ss;
        ss << path << "assemblies/" << partnames[i] << "/thumb.tga";
        std::cout << ss.str() << "\n";
        GLFWimage img;
        if (glfwReadImage(ss.str().c_str(), &img, 0))
        {
            thumbnails.push_back(makeTexture(img));
            glfwFreeImage(&img);
        }
        else
        {
            thumbnails.push_back(0);
            std::cout << "Could not load thumbnail for " << partnames[i] << "\n";
        }
    }
    cursor = textureFromFile("cursor.tga");
    panel = ninePatch(textureFromFile("9patch.tga"));
    button = ninePatch(textureFromFile("button.tga"));
    bubble = ninePatch(textureFromFile("bubble.tga"), 8, 16, 12, 8, 0.25, 0.5, 0.375, 0.75);
    tooltextures.push_back(textureFromFile("drag.tga"));
    tooltextures.push_back(textureFromFile("axis.tga"));
    tooltextures.push_back(textureFromFile("delete.tga"));
    font = textureFromFile("font.tga");
    cursorx = 0;
    cursory = 0;
    mousevelx = 0;
    mousevely = 0;
    mouseWasCaptured = false;
    camera.position = btVector3(0, 0, 10);
    camera.pitch = 0;
    camera.yaw = 0;
    camera.orientationFromAngles();
    selectedItem = -1;
    selectedTool = -1;
    static unsigned char fontpixels[STB_SOMEFONT_BITMAP_HEIGHT][STB_SOMEFONT_BITMAP_WIDTH];
    STB_SOMEFONT_CREATE(fontdata, fontpixels, STB_SOMEFONT_BITMAP_HEIGHT);
    glGenTextures(1, &stbfont);
    glBindTexture(GL_TEXTURE_2D, stbfont);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_WRAP_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_WRAP_BORDER);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, STB_SOMEFONT_BITMAP_HEIGHT, STB_SOMEFONT_BITMAP_WIDTH, 0, GL_ALPHA, GL_UNSIGNED_BYTE, fontpixels);
}

void workshopScene::update(sceneInfo &info)
{
    gWorld->btWorld->stepSimulation(1.f/60.f);

    mousevelx = mousevelx * 0.95f + info.dmousex * 0.2f;
    mousevely = mousevely * 0.95f + info.dmousey * 0.2f;

    if (info.keys.held.MouseM)
    {
        camera.pitch -= info.dmousey * 0.02;
        camera.yaw -= info.dmousex * 0.02;
        camera.orientationFromAngles();
    }

    sceneInfo::keyState &keys = info.keys;
    if (!glfwGetKey('E'))
    {
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
    }
    if (info.captureMouse)
    {
        if (mouseWasCaptured)
        {
            cursorx += info.dmousex;
            cursory += info.dmousey;
            if (cursorx < 0)
                cursorx = 0;
            if (cursorx > info.width)
                cursorx = info.width;
            if (cursory < 0)
                cursory = 0;
            if (cursory > info.height)
                cursory = info.height;
        }
        else
        {
            cursorx = info.lastmousex;
            cursory = info.lastmousey;
        }
    }
    else if (mouseWasCaptured)
    {
        glfwSetMousePos(cursorx, cursory);
    }
    mouseWasCaptured = info.captureMouse;

    if (info.keys.held.space && selectedItem < partnames.size())
    {
        int initialcount = gWorld->objects.size();
        loadAssembly(partnames[selectedItem], btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 10, 0)), path, gWorld);
        for (unsigned int i = initialcount; i < gWorld->objects.size(); i++)
            gWorld->objects[i]->body->setDamping(0.95f, 0.96f);
    }

    if (cursorx < UI_SIZE)
    {
        if (keys.newPress.MouseL)
        {
            selectedItem = (cursory - 4) / (UI_SIZE + 8);
            selectedTool = -1;
        }
    }
    else if (cursory > info.height - UI_SMALL - 16)
    {
        if (keys.newPress.MouseL)
        {
            selectedTool = (cursorx - UI_SIZE - 16 - 8) / (UI_SMALL + 8);
            selectedItem = -1;
        }
    }
    else
    {
        mouseRayDir = camera.forward + camera.right * (cursorx - info.width / 2) / (float)info.height * 2 - camera.up * (cursory - info.height / 2) / (float)info.height * 2;
        btVector3 raystart = camera.position;
        btVector3 rayend = raystart + mouseRayDir * 100;
        mouseRayCallback = btCollisionWorld::ClosestRayResultCallback(raystart, rayend);
        gWorld->btWorld->rayTest(raystart, rayend, mouseRayCallback);

        if (keys.newPress.MouseL && mouseRayCallback.hasHit())
        {
            mouseHeldBody = (btRigidBody*)mouseRayCallback.m_collisionObject;
            if (selectedItem >= 0)
            {
                if (selectedItem < partnames.size())
                {
                    int initialcount = gWorld->objects.size();
                    btTransform trans(btQuaternion(0, 0, 0, 1), mouseRayCallback.m_hitPointWorld + mouseRayCallback.m_hitNormalWorld * 1.f);
                    loadAssembly(partnames[selectedItem], trans, path, gWorld);
                    for (unsigned int i = initialcount; i < gWorld->objects.size(); i++)
                        gWorld->objects[i]->body->setDamping(0.95f, 0.96f);
                }
            }
            else if (selectedTool == 0)
            {
                mousePerpDist = camera.forward.dot(mouseRayCallback.m_hitPointWorld - raystart);
                mouseHeldBody->setDamping(0.995, 0.98);
                mouseHeldBody->activate();
                btVector3 localPivot = mouseHeldBody->getCenterOfMassTransform().inverse() * mouseRayCallback.m_hitPointWorld;
                mouseConstraint = new btGeneric6DofConstraint(*mouseHeldBody, btTransform(btQuaternion(0, 0, 0, 1), localPivot), false);
                if (glfwGetKey('E'))
                {
                     mouseConstraint->setAngularLowerLimit(btVector3(0, 0, 0));
                    mouseConstraint->setAngularUpperLimit(btVector3(0, 0, 0));
                }
                gWorld->btWorld->addConstraint(mouseConstraint);
            }
            else if (selectedTool == 1)
            {
                if (!mouseHeldBody->isStaticObject())
                {
                    if (!axisHasFirst)
                    {
                        axisHasFirst = true;
                        axisResult = mouseRayCallback;
                        axisFirstPivot = mouseHeldBody->getCenterOfMassTransform().inverse() * axisResult.m_hitPointWorld;
                        axisFirstNormal = btTransform(mouseHeldBody->getCenterOfMassTransform().inverse().getRotation(), btVector3(0, 0, 0)) * axisResult.m_hitNormalWorld;
                        std::cout << "First point for axis.\n";
                    }
                    else
                    {
                        btVector3 axisSecondNormal = btTransform(mouseHeldBody->getCenterOfMassTransform().inverse().getRotation(), btVector3(0, 0, 0)) * mouseRayCallback.m_hitNormalWorld;
                        btVector3 axisSecondPivot = mouseHeldBody->getCenterOfMassTransform().inverse() * mouseRayCallback.m_hitPointWorld + axisSecondNormal * 0.05;
                        gWorld->addConstraint(new btHingeConstraint(*(btRigidBody*)axisResult.m_collisionObject, *mouseHeldBody, axisFirstPivot, axisSecondPivot, -axisFirstNormal, axisSecondNormal));
                        mouseHeldBody->activate();
                        axisHasFirst = false;
                    }
                }
            }
            else if (selectedTool == 2)
            {
                btRigidBody *body = (btRigidBody*)mouseRayCallback.m_collisionObject;
                if (!body->isStaticObject())
                    gWorld->removeBody(body);
            }
        }
    }

    if (mouseConstraint)
    {
        if (keys.held.MouseL)
        {
            mousePerpDist *= pow(1.1, keys.dmouseWheel);
            mouseConstraint->getFrameOffsetA().setOrigin(camera.position + mouseRayDir * mousePerpDist);        // note that raydir is not unit length: it stretches from the camera to the near plane.
        }
        else
        {
            mouseHeldBody->setDamping(0.95, 0.96);
            gWorld->btWorld->removeConstraint(mouseConstraint);
            delete mouseConstraint;
            mouseConstraint = 0;
        }
        if (glfwGetKey('E'))
        {
            btTransform invrotate;
            mouseHeldBody->getMotionState()->getWorldTransform(invrotate);
            invrotate = invrotate.inverse();
            if (keys.held.W)
                mouseHeldBody->applyImpulse(invrotate * camera.forward, invrotate * camera.up);
            if (keys.held.S)
                mouseHeldBody->applyImpulse(-camera.forward, camera.up);
            if (keys.held.A)
                mouseHeldBody->applyImpulse(-camera.right, camera.forward);
            if (keys.held.D)
                mouseHeldBody->applyImpulse(camera.right, camera.forward);
        }
    }

    if (axisHasFirst && selectedTool != 1)
        axisHasFirst = false;
}

void texturedQuad(float left, float right, float bottom, float top, float tleft = 0.f, float tright = 1.f, float tbottom = 0.f, float ttop = 1.f)
{
    glBegin(GL_TRIANGLES);
    glTexCoord2f(tleft, tbottom);
    glVertex3f(left, bottom, -1.f);
    glTexCoord2f(tleft, ttop);
    glVertex3f(left, top, -1.f);
    glTexCoord2f(tright, ttop);
    glVertex3f(right, top, -1.f);
    glTexCoord2f(tleft, tbottom);
    glVertex3f(left, bottom, -1.f);
    glTexCoord2f(tright, ttop);
    glVertex3f(right, top, -1.f);
    glTexCoord2f(tright, tbottom);
    glVertex3f(right, bottom, -1.f);
    glEnd();
}

ninePatch::ninePatch(GLuint texture_, int soffsleft_, int soffsright_, int soffsbottom_, int soffstop_, float toffsleft_, float toffsright_, float toffsbottom_, float toffstop_)
{
    texture = texture_;
    soffsleft = soffsleft_;
    soffsright = soffsright_;
    soffsbottom = soffsbottom_;
    soffstop = soffstop_;
    toffsleft = toffsleft_;
    toffsright = toffsright_;
    toffsbottom = toffsbottom_;
    toffstop = toffstop_;
    int currenttexture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &currenttexture);
    if (currenttexture != texture)
        glBindTexture(GL_TEXTURE_2D, texture);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
    std::cout << width << ", " << height << "\n";
}

void ninePatch::draw(float left, float right, float bottom, float top)
{
    int currenttexture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &currenttexture);
    if (currenttexture != texture)
        glBindTexture(GL_TEXTURE_2D, texture);
    texturedQuad(left, left + soffsleft, bottom, bottom - soffsbottom, 0, toffsleft, 0, toffsbottom);
    texturedQuad(left + soffsleft, right - soffsright, bottom, bottom - soffsbottom, toffsleft, toffsright, 0, toffsbottom);
    texturedQuad(right - soffsright, right, bottom, bottom - soffsbottom, toffsright, 1.0, 0, toffsbottom);
    texturedQuad(left, left + soffsleft, bottom - soffsbottom, top + soffstop, 0, toffsleft, toffsbottom, toffstop);
    texturedQuad(left + soffsleft, right - soffsright, bottom - soffsbottom, top + soffstop, toffsleft, toffsright, toffsbottom, toffstop);
    texturedQuad(right - soffsright, right, bottom - soffsbottom, top + soffstop, toffsright, 1.0, toffsbottom, toffstop);
    texturedQuad(left, left + soffsleft, top + soffstop, top, 0, toffsleft, toffstop, 1);
    texturedQuad(left + soffsleft, right - soffsright, top + soffstop, top, toffsleft, toffsright, toffstop, 1);
    texturedQuad(right - soffsright, right, top + soffstop, top, toffsright, 1.0, toffstop, 1);
}

void workshopScene::renderChar(int x, int y, char c)
{
    texturedQuad(x, x + 8, y, y + 8, 0.f, 1.f, 1.f - c / 128.f, 1.f - (c + 1) / 128.f);
}

void workshopScene::print(int x, int y, std::string str)
{
    glBindTexture(GL_TEXTURE_2D, font);
    for (unsigned int i = 0; i < str.size(); i++)
    {
        renderChar(x + i * 8, y, str[i]);
    }
}

void workshopScene::stbprint(int x, int y, char *str)
{
    glBindTexture(GL_TEXTURE_2D, stbfont);

    glBegin(GL_QUADS);
    while (*str) {
        int char_codepoint = *str++;
        stb_fontchar *cd = &fontdata[char_codepoint - STB_SOMEFONT_FIRST_CHAR];
        glTexCoord2f(cd->s0, cd->t0); glVertex2i(x + cd->x0, y + cd->y0);
        glTexCoord2f(cd->s1, cd->t0); glVertex2i(x + cd->x1, y + cd->y0);
        glTexCoord2f(cd->s1, cd->t1); glVertex2i(x + cd->x1, y + cd->y1);
        glTexCoord2f(cd->s0, cd->t1); glVertex2i(x + cd->x0, y + cd->y1);
        x += cd->advance_int;
    }
    glEnd();
}

void workshopScene::showBubble(int x, int y, std::string str)
{
    glColor3f(1, 1, 1);
    bubble.draw(x - 8 * (str.size() - 1) - bubble.soffsleft, x + bubble.soffsright, y - 4 + bubble.soffsbottom, y - 12 - bubble.soffstop);
    glColor3f(0, 0, 0);
    print(x - 8 * (str.size() - 1), y - 12, str);
}

void workshopScene::render(sceneInfo &info)
{
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glFrustum((-1.f * info.width) / info.height, (1.f * info.width) / info.height, -1.f, 1.f, 1.f, 1000.f);

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

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
    glColor3f(1.f, 1.f, 1.f);

    glEnable(GL_CULL_FACE);

    gWorld->render();

    if (axisHasFirst)
    {
        glColor3f(1, 0, 0);
        btTransform trans = ((btRigidBody*)(axisResult.m_collisionObject))->getCenterOfMassTransform();
        btVector3 start = trans * axisFirstPivot;
        btVector3 end = trans * (axisFirstPivot + axisFirstNormal * 0.5);
        glBegin(GL_LINES);
        glVertex3f(start.getX(), start.getY(), start.getZ());
        glVertex3f(end.getX(), end.getY(), end.getZ());
        glEnd();

    }

    glPushAttrib(GL_ENABLE_BIT);

    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.f, info.width, info.height, 0.f, -1.f, 1.f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor3f(1.f, 1.f, 1.f);

    panel.draw(0, UI_SIZE + 16, info.height, 0);

    glPushAttrib(GL_SCISSOR_BIT);
    glEnable(GL_SCISSOR_TEST);
    glScissor(8, 8, UI_SIZE, info.height - 16);
    for (unsigned int i = 0; i < thumbnails.size(); i++)
    {
        int basex = 8;
        int basey = 8 + (UI_SIZE + 8) * i;
        if (i == selectedItem)
            glColor3f(1.f, 1.f, 0.5f);
        else
            glColor3f(1.f, 1.f, 1.f);

        button.draw(basex, basex + UI_SIZE, basey + UI_SIZE, basey);
        glBindTexture(GL_TEXTURE_2D, thumbnails[i]);
        texturedQuad(basex + 8, basex + (UI_SIZE - 8), basey + (UI_SIZE - 8), basey + 8);
    }
    glPopAttrib();
    panel.draw(UI_SIZE + 16, info.width, info.height, info.height - UI_SMALL - 16);
    glColor3f(1.f, 1.f, 1.f);
    for (unsigned int i = 0; i < tooltextures.size(); i++)
    {
        int basex = UI_SIZE + 16 + 8 + (UI_SMALL + 8) * i;
        int basey = info.height - 8 - UI_SMALL;
        if (i == selectedTool)
            glColor3f(1.f, 1.f, 0.5f);
        else
            glColor3f(1.f, 1.f, 1.f);
        button.draw(basex, basex + UI_SMALL, basey + UI_SMALL, basey);
        glBindTexture(GL_TEXTURE_2D, tooltextures[i]);
        texturedQuad(basex + 8, basex + UI_SMALL - 8, basey + UI_SMALL - 8, basey + 8);
    }
    glColor3f(1.f, 1.f, 1.f);
    if (info.captureMouse)
    {
        if (mousevelx * mousevelx + mousevely * mousevely < 1.f)
            glColor3f(1, 0, 0);
        glBindTexture(GL_TEXTURE_2D, cursor);
        texturedQuad(cursorx, cursorx + 20, cursory + 20, cursory);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBegin(GL_LINES);
        glVertex2f(cursorx, cursory);
        glVertex2f(cursorx + mousevelx, cursory + mousevely);
        glEnd();

    }

    if (mousevelx * mousevelx + mousevely * mousevely < 1.f && mouseRayCallback.hasHit())
        showBubble(cursorx, cursory, gWorld->findObj((btRigidBody*)mouseRayCallback.m_collisionObject)->tag);


    /*glBindTexture(GL_TEXTURE_2D, font);
    info.keybuffer.push_back(0);
    print(100, 100, &info.keybuffer[0]);
    glBindTexture(GL_TEXTURE_2D, stbfont);
    texturedQuad(100, 200, 500, 400);
    stbprint(100, 300, &info.keybuffer[0]);
    info.keybuffer.erase(info.keybuffer.end() - 1);*/

    glPopAttrib();
}

void workshopScene::camera::orientationFromAngles()
{
    orientation.setEuler(yaw, pitch, 0);
    btQuaternion qforward = orientation * btQuaternion(0, 0, -1, 0) * orientation.inverse();
    forward = btVector3(qforward.getX(), qforward.getY(), qforward.getZ());
    btQuaternion qright = orientation * btQuaternion(1, 0, 0, 0) * orientation.inverse();
    right = btVector3(qright.getX(), qright.getY(), qright.getZ());
    btQuaternion qup = orientation * btQuaternion(0, 1, 0, 0) * orientation.inverse();
    up = btVector3(qup.getX(), qup.getY(), qup.getZ());
}

GLuint workshopScene::textureFromFile(std::string name, std::string reldir)
{
    std::stringstream ss;
    ss << path << reldir << name;
    GLFWimage img;
    if (!glfwReadImage(ss.str().c_str(), &img, 0))
        std::cout << "Could not load texture: " << ss.str() << "\n";
    GLuint texhandle = makeTexture(img);
    glfwFreeImage(&img);
    return texhandle;
}

workshopScene::~workshopScene()
{
    delete gWorld;
}
