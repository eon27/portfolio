/*
 *  Workshop 1
 *  Computer Graphics course
 *  Dept Computing Science, Umea University
 *  Stefan Johansson, stefanj@cs.umu.se
 */

#include "geometryrender.h"
#include "glfwcallbackmanager.h"

OpenGLWindow* glfwCallbackManager::app = nullptr;

int main(int argc, char **argv)
{

    GeometryRender app("Workshop 1", 1024, 768);
    glfwCallbackManager::initCallbacks(&app);
    app.initialize();
    
    app.start();
}
