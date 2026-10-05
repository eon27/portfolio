/*
 *  Workshop 1
 *  Computer Graphics course
 *  Dept Computing Science, Umea University
 *  Stefan Johansson, stefanj@cs.umu.se
 */
#pragma once

#include "openglwindow.h"
#include "mycamera.hpp"

class GeometryRender : public OpenGLWindow
{
public:
    template<typename... ARGS>
    GeometryRender(ARGS&&... args) : OpenGLWindow{ std::forward<ARGS>(args)... }
    {}

    void initialize();
    virtual void display() override;
    virtual void controlls(int action) override;
    virtual void rotateCamera(float deltaX, float deltaY) override;
    virtual void moveCamera(int action, float deltaTime) override;
    virtual void handleNewObject() override;
    virtual void handleNewTexture(unsigned char *data, int width, int height, int nrChannels) override;
    virtual void newShader() override;

private:

    #define SHADERS 3
    GLuint program[SHADERS];

    // OpenGL buffers
    #define OBJECTS 1
    GLuint vao[OBJECTS];
    GLuint vBuffer[OBJECTS];
    GLuint iBuffer[OBJECTS];
    
    // OpenGL attribute locations
    GLuint locVertices[SHADERS];// = {0, 0, 0}; // Both shaders use location 0 for vPosition
    GLuint locNormals[SHADERS];// = {1, 1, 1};   // Both shaders use location 1 for vNormal
    GLuint locTexturePos[SHADERS] ;//= {2, 2, 2}; // Both shaders use location 2 for vTexture
    
    GLuint locModel[SHADERS];
    GLuint locView[SHADERS];
    GLuint locProjection[SHADERS];

    GLuint locCameraPos[SHADERS];

    GLuint locLightPos[SHADERS];
    GLuint locLightColor[SHADERS];
    GLuint locAmbientColor[SHADERS];

    GLuint locMaterialAmbient[SHADERS];
    GLuint locMaterialDiffuse[SHADERS];
    GLuint locMaterialSpecular[SHADERS];
    GLuint locMaterialShininess[SHADERS];

    GLuint locUseTexture[SHADERS];

    // Geometry data
    std::vector<Vector3> vertexList;
    std::vector<Vector3> normalList;
    std::vector<unsigned int> indexList;
    std::vector<Vector2> texCoords;

    float moveObject;
    float pointMaxDistance;

    void debugShader(void) const;

    void centerAndScaleObject();
    void loadGeometry();
    void loadObjectGeometry();

    void updateView();

    MyCamera cam;

    Matrix matModel = Matrix();
};
