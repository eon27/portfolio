/*
 *  Workshop 1
 *  Computer Graphics course
 *  Dept Computing Science, Umea University
 *  Stefan Johansson, stefanj@cs.umu.se
 */

#include "geometryrender.h"
#include "mycamera.hpp"

#define CAMERA_MOVE_SPEED 1
#define OBJECT_MOVE_SPEED 0.1

using namespace std;

// Initialize OpenGL
void GeometryRender::initialize()
{
    // Enable depth test
    glEnable(GL_DEPTH_TEST);

    // Create and initialize a program object with shaders
    program[0] = initProgram("shader/vshader.glsl", "shader/fshader.glsl");
    program[1] = initProgram("shader/vshaderGouraud.glsl", "shader/fshaderGouraud.glsl");
    program[2] = initProgram("shader/vshaderWireframe.glsl", "shader/fshaderWireframe.glsl");
    // Installs the program object as part of current rendering state
    glUseProgram(program[0]);

    // Creat a vertex array object
    glGenVertexArrays(OBJECTS, vao);
    glBindVertexArray(vao[0]);

    // Create vertex buffer in the shared display list space and
    // bind it as VertexBuffer (GL_ARRAY_BUFFER)
    glGenBuffers(OBJECTS, vBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vBuffer[0]);

    /* Create buffer in the shared display list space and 
       bind it as GL_ELEMENT_ARRAY_BUFFER */
    glGenBuffers(OBJECTS, iBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iBuffer[0]);

    // Get locations of the attributes in the shader
    locVertices[0] = glGetAttribLocation( program[0], "vPosition");
    locNormals[0] = glGetAttribLocation(program[0], "vNormal");
    locTexturePos[0] = glGetAttribLocation(program[0], "vTexture");
    
    locModel[0] = glGetUniformLocation(program[0],"M");
    locView[0] = glGetUniformLocation(program[0],"V");
    locProjection[0] = glGetUniformLocation(program[0],"P");

    locCameraPos[0] = glGetUniformLocation(program[0],"camPos");

    locLightPos[0] = glGetUniformLocation(program[0],"lightPos");
    locAmbientColor[0] = glGetUniformLocation(program[0],"ambientColor");
    locLightColor[0] = glGetUniformLocation(program[0],"lightColor");

    locMaterialAmbient[0] = glGetUniformLocation(program[0],"materialAmbient");
    locMaterialDiffuse[0] = glGetUniformLocation(program[0],"materialDiffuse");
    locMaterialSpecular[0] = glGetUniformLocation(program[0],"materialSpecular");
    locMaterialShininess[0] = glGetUniformLocation(program[0],"materialShininess");

    locUseTexture[0] = glGetUniformLocation(program[0], "useTexture");

    glUseProgram(program[1]);

    locVertices[1] = glGetAttribLocation( program[1], "vPosition");
    locNormals[1] = glGetAttribLocation(program[1], "vNormal");
    locTexturePos[1] = glGetAttribLocation(program[1], "vTexture");
    
    locModel[1] = glGetUniformLocation(program[1],"M");
    locView[1] = glGetUniformLocation(program[1],"V");
    locProjection[1] = glGetUniformLocation(program[1],"P");

    locCameraPos[1] = glGetUniformLocation(program[1],"camPos");

    locLightPos[1] = glGetUniformLocation(program[1],"lightPos");
    locAmbientColor[1] = glGetUniformLocation(program[1],"ambientColor");
    locLightColor[1] = glGetUniformLocation(program[1],"lightColor");

    locMaterialAmbient[1] = glGetUniformLocation(program[1],"materialAmbient");
    locMaterialDiffuse[1] = glGetUniformLocation(program[1],"materialDiffuse");
    locMaterialSpecular[1] = glGetUniformLocation(program[1],"materialSpecular");
    locMaterialShininess[1] = glGetUniformLocation(program[1],"materialShininess");

    locUseTexture[1] = glGetUniformLocation(program[1], "useTexture");

    glUseProgram(program[2]);

    locVertices[2] = glGetAttribLocation( program[2], "vPosition");
    locNormals[2] = glGetAttribLocation(program[2], "vNormal"); // Not used but needed in loadGeometry
    locTexturePos[2] = glGetAttribLocation(program[2], "vTexture"); // Not used but needed in loadGeometry

    locModel[2] = glGetUniformLocation(program[2],"M");
    locView[2] = glGetUniformLocation(program[2],"V");
    locProjection[2] = glGetUniformLocation(program[2],"P");

    glBindVertexArray(0);
    glUseProgram(0);

    cam = MyCamera(500, 60);
}

/**
 * Send the shader inputs with buffers for the vertices, normals, texture coordinates and face indices
 */
void GeometryRender::loadGeometry()
{  
    // Get the number of bytes for each buffer
    size_t vSize = vertexList.size()*sizeof(float)*3;
    size_t nSize = normalList.size()*sizeof(float)*3;
    size_t tSize = texCoords.size()*sizeof(float)*2;
    size_t iSize = indexList.size()*sizeof(unsigned int);
    

    glUseProgram(program[shading_current_idx]);
    glBindVertexArray(vao[0]);

    // Set the pointers of locVertices to the right places
    glVertexAttribPointer(locVertices[shading_current_idx], 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glEnableVertexAttribArray(locVertices[shading_current_idx]);

    glVertexAttribPointer(locNormals[shading_current_idx], 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(vSize));
    glEnableVertexAttribArray(locNormals[shading_current_idx]);

    glVertexAttribPointer(locTexturePos[shading_current_idx], 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(vSize + nSize));
    glEnableVertexAttribArray(locTexturePos[shading_current_idx]);
    
    // Send the matrix uniforms
    glUniformMatrix4fv(locModel[shading_current_idx], 1, GL_TRUE, matModel.mat);
    glUniformMatrix4fv(locView[shading_current_idx], 1, GL_TRUE, cam.getViewMatrix().mat);
    glUniformMatrix4fv(locProjection[shading_current_idx], 1, GL_TRUE, cam.getProjectionMatrix().mat);

    // Load object data to the array buffer and index array
    glBufferData(GL_ARRAY_BUFFER, vSize + nSize + tSize, NULL, GL_STATIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER, 0, vSize, vertexList.data()->vec);
    glBufferSubData(GL_ARRAY_BUFFER, vSize, nSize, normalList.data()->vec);
    glBufferSubData(GL_ARRAY_BUFFER, vSize + nSize, tSize, texCoords.data()->vec);
    
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, iSize, indexList.data(), GL_STATIC_DRAW );

    glBindVertexArray(0);
    glUseProgram(0);
}

/**
 * Fit object inside NDC cube
 */
void GeometryRender::centerAndScaleObject() {
	if (vertexList.empty()) return;
	// Reset the matrix	
	matModel = Matrix();

	float lowestX = vertexList[0].vec[0];
	float lowestY = vertexList[0].vec[1];
	float lowestZ = vertexList[0].vec[2];

	float highestX = vertexList[0].vec[0];
	float highestY = vertexList[0].vec[1];
	float highestZ = vertexList[0].vec[2];

	// Find edge points in the x,y,z directions
	for (long unsigned int i = 0; i < vertexList.size(); i++) {
		lowestX = min<float>(lowestX, vertexList[i].vec[0]);
		lowestY = min<float>(lowestY, vertexList[i].vec[1]);
		lowestZ = min<float>(lowestZ, vertexList[i].vec[2]);
		
		highestX = max<float>(highestX, vertexList[i].vec[0]);
		highestY = max<float>(highestY, vertexList[i].vec[1]);
		highestZ = max<float>(highestZ, vertexList[i].vec[2]);
	}

    // Middle is between the edges
	float middleX = (lowestX + highestX) / 2;
	float middleY = (lowestY + highestY) / 2;
	float middleZ = (lowestZ + highestZ) / 2;
	
	// Length of object in x,y,z directions
	float lengthX = highestX - lowestX;
	float lengthY = highestY - lowestY;
	float lengthZ = highestZ - lowestZ;
	
	// Avoid division by 0, (0 == false so !0 == true)
	if (!lengthX) lengthX = 1;
	if (!lengthY) lengthY = 1;
	if (!lengthZ) lengthZ = 1;
	
	// Find the max length of the object and scale it down to length 1
	float longestLength = max<float>(lengthX, max<float>(lengthY, lengthZ));
	float shrink = 1/longestLength;

    // Maximum possible distance from a point on the object to the origin (used in texture coordinate calculation, to avoid looping over vertices twice)
    pointMaxDistance = longestLength/2 * sqrt(2);
    
	matModel.scale(shrink, shrink, shrink);
    moveObject = longestLength * OBJECT_MOVE_SPEED; // Scale the movement with the shrinkage of the object.
    
    // Move all points such that the middle of the object is at origin (0,0,0)
    for (long unsigned int i = 0; i < vertexList.size(); i++) {
		vertexList[i].vec[0] -= middleX;
		vertexList[i].vec[1] -= middleY;
		vertexList[i].vec[2] -= middleZ;
	}
}

// Check if any error has been reported from the shader
void GeometryRender::debugShader(void) const
{
    GLint  logSize;
    glGetProgramiv( program[0], GL_INFO_LOG_LENGTH, &logSize );
    if (logSize > 0) {
        std::cerr << "Failure in shader "  << std::endl;
        char logMsg[logSize+1];
        glGetProgramInfoLog( program[0], logSize, nullptr, &(logMsg[0]) );
        std::cerr << "Shader info log: " << logMsg << std::endl;
    }

}

// Render object
void GeometryRender::display()
{
    
    glUseProgram(program[shading_current_idx]);
    glBindVertexArray(vao[0]);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (shading_current_idx == 2) {
        // Wireframe shader
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        glDrawElements(GL_TRIANGLES, static_cast<int>(indexList.size()), GL_UNSIGNED_INT, BUFFER_OFFSET(0));
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    } else {
           
        updateView();
        
        // Call OpenGL to draw the triangle
        glDrawElements(GL_TRIANGLES, static_cast<int>(indexList.size()), GL_UNSIGNED_INT, BUFFER_OFFSET(0));
    }

    glBindVertexArray(0);
    glUseProgram(0);
}

/**
 * Takes the inputs from the GUI and utdates the values in the shader.
 */
void GeometryRender::updateView() {

    // Must treat them as floats to so result doesn't get rounded
    float aspectRatio = ((float) width()) / ((float) height());

    cam.updateView(fov, farplane, top, obliqueScale, obliqueAngleRad, proj_current_idx, aspectRatio);
    glUniformMatrix4fv(locProjection[shading_current_idx], 1, GL_TRUE, cam.getProjectionMatrix().mat);

    glUniform3f(locCameraPos[shading_current_idx], cam.getPosition().vec[0], cam.getPosition().vec[1], cam.getPosition().vec[2]);

    glUniform3f(locLightPos[shading_current_idx], lightPos[0], lightPos[1], lightPos[2]);
    glUniform3f(locLightColor[shading_current_idx], lightColor[0], lightColor[1], lightColor[2]);
    glUniform3f(locAmbientColor[shading_current_idx], ambientColor[0], ambientColor[1], ambientColor[2]);

    glUniform3f(locMaterialAmbient[shading_current_idx], materialAmbient[0], materialAmbient[1], materialAmbient[2]);
    glUniform3f(locMaterialDiffuse[shading_current_idx], materialDiffuse[0], materialDiffuse[1], materialDiffuse[2]);
    glUniform3f(locMaterialSpecular[shading_current_idx], materialSpecular[0], materialSpecular[1], materialSpecular[2]);
    glUniform1f(locMaterialShininess[shading_current_idx], materialShininess);

    glUniform1i(locUseTexture[shading_current_idx], textureShow);
}

/**
 * Take an input and transform the object accordingly
 */
void GeometryRender::controlls(int action) {
    glUseProgram(program[shading_current_idx]);
    switch (action)
    {
    // Rotating the object
    case GLFW_KEY_UP:
        matModel.rotatex(-M_PI/18);
        break;
    case GLFW_KEY_DOWN:
        matModel.rotatex(M_PI/18);
        break;
    case GLFW_KEY_LEFT:
        matModel.rotatey(M_PI/18);
        break;
    case GLFW_KEY_RIGHT:
        matModel.rotatey(-M_PI/18);
        break;
    // Moving the object in 3d
    case GLFW_KEY_J:
        matModel.translate(-moveObject, 0, 0);
        break;
    case GLFW_KEY_L:
        matModel.translate(moveObject, 0, 0);
        break;
    case GLFW_KEY_I:
        matModel.translate(0, moveObject, 0);
        break;
    case GLFW_KEY_K:
        matModel.translate(0, -moveObject, 0);
        break;
    case GLFW_KEY_N:
        matModel.translate(0, 0, moveObject);
        break;
    case GLFW_KEY_M:
        matModel.translate(0, 0, -moveObject);
        break;
    default:
        break;
    }
    glUniformMatrix4fv(locModel[shading_current_idx], 1, GL_TRUE, matModel.mat);
}

/**
 * Rotate the camera and update the shader parameter
 */
void GeometryRender::rotateCamera(float deltaX, float deltaY) {
    glUseProgram(program[shading_current_idx]);
    cam.rotate(deltaX/180, deltaY/180);
    glUniformMatrix4fv(locView[shading_current_idx], 1, GL_TRUE, cam.getViewMatrix().mat);
    glUseProgram(0);
}

/**
 * Move the camera and update the shader parameter
 */
void GeometryRender::moveCamera(int action, float deltaTIme) {
    glUseProgram(program[shading_current_idx]);
    switch (action)
    {
    // Moving the camera in 3d
    case GLFW_KEY_W:
        cam.move(0,0,CAMERA_MOVE_SPEED*deltaTIme);
        break;
    case GLFW_KEY_S:
        cam.move(0,0,-CAMERA_MOVE_SPEED*deltaTIme);
        break;
    case GLFW_KEY_A:
        cam.move(-CAMERA_MOVE_SPEED*deltaTIme,0,0);
        break;
    case GLFW_KEY_D:
        cam.move(CAMERA_MOVE_SPEED*deltaTIme,0,0);
        break;
    case GLFW_KEY_Q:
        cam.move(0,-CAMERA_MOVE_SPEED*deltaTIme,0);
        break;
    case GLFW_KEY_E:
        cam.move(0,CAMERA_MOVE_SPEED*deltaTIme,0);
        break;
    }
    glUniformMatrix4fv(locView[shading_current_idx], 1, GL_TRUE, cam.getViewMatrix().mat);
    glUseProgram(0);
}


void GeometryRender::handleNewObject() {
    vertexList.clear();
    indexList.clear();
    normalList.clear();
    texCoords.clear();

    // Load vertices
    for (int i = 0; i < objData.vertexCount; i++) {
        vertexList.push_back(Vector3(objData.vertexList[i][0].e));
    }
    
    // Load indices
    for (int i = 0; i < objData.faceCount; i++) {
        for (int j = 2; j < objData.faceList[i]->vertex_count; j++) {
            indexList.push_back(objData.faceList[i]->vertex_index[0]);
            indexList.push_back(objData.faceList[i]->vertex_index[j-1]);
            indexList.push_back(objData.faceList[i]->vertex_index[j]);
        }
    }
    
    // Add empty normals for each vertex
    for (int i = 0; i < objData.vertexCount; i++) {
        normalList.push_back(Vector3(0,0,0));
    }

    // Check if the obj file had face normals
    if (objData.normalCount) {
        for (int i = 0; i < objData.faceCount; i++) {
            // For face i, go trough all vertices in the face.
            for (int j = 0; j < objData.faceList[i]->vertex_count; j++) {

                // Get the index of vertex j in face i
                int currentVertex = objData.faceList[i]->vertex_index[j];
                // Find which of the normals face i uses
                int normalIndexOfVertex = objData.faceList[i]->normal_index[j];
                
                // Add the face normal to the vertex normal
                normalList[currentVertex] = normalList[currentVertex] + Vector3(objData.normalList[normalIndexOfVertex]->e).normalize();
            }
        }
    } else {
        // Calculate vertex normals. To account for quads, the indexList and vertexList must be used insted of the 
        for (size_t i = 0; i < indexList.size(); i += 3) {
            // Construct two vector in the face
            Vector3 vector1 = vertexList[indexList[i]] - vertexList[indexList[i+1]];
            Vector3 vector2 = vertexList[indexList[i]] - vertexList[indexList[i+2]];

            Vector3 normal = vector1.cross(vector2).normalize();

            // Add the normal to all the vertices of the face
            normalList[indexList[i]] = normalList[indexList[i]] + normal;
            normalList[indexList[i+1]] = normalList[indexList[i+1]] + normal;
            normalList[indexList[i+2]] = normalList[indexList[i+2]] + normal;
        }
    }

    //Normalize the normal values
    for (size_t i = 0; i < normalList.size(); i++){
        normalList[i] = normalList[i].normalize();
    }
    
    // Make object fit inside NDC cube
    centerAndScaleObject();
    
    // Calculate texture coordinates for vertices, by using two part mapping with sphere
    for (size_t i = 0; i < vertexList.size(); i++) {
        // float b = 2 * (vertexList[i] * normalList[i]);
        // float c = (vertexList[i] * vertexList[i]) - pointMaxDistance * pointMaxDistance; // pointMaxDistance = maximum possible distance from vertex to origin 
        // float q = -0.5 * (b + (signbit(b) * 2 - 1) * sqrt(b*b - 4*c)); // Signbit = true/false (1/0), 1 * 2 - 1 = 1, 0 * 2 - 1 = -1

        // float d1 = q;
        // float d2 = c/q;

        // float dPlus = d1;
        // if (d2 > d1) dPlus = d2;

        // Vector3 intersect = vertexList[i] + normalList[i] * dPlus;
        // intersect = intersect.normalize();

        // float s = 0.5 - asin(intersect.vec[1] / pointMaxDistance) / (M_PI);
        // float t = atan2(intersect.vec[2] / pointMaxDistance, intersect.vec[0] / pointMaxDistance) / (2 * M_PI) + 0.5;
        // texCoords.push_back(Vector2(t, s));

        float b = 2 * (vertexList[i] * normalList[i]);
        float c = (vertexList[i] * vertexList[i]) - pointMaxDistance * pointMaxDistance; // pointMaxDistance = maximum possible distance from vertex to origin 
        float q = -0.5 * (b + (signbit(b) * 2 - 1) * sqrt(b*b - 4*c)); // Signbit = true/false (1/0), 1 * 2 - 1 = 1, 0 * 2 - 1 = -1

        float d1 = q;
        float d2 = c/q;

        float dPlus = d1;
        if (d2 > d1) dPlus = d2;

        Vector3 intersect = vertexList[i] + normalList[i] * dPlus;
        intersect = intersect.normalize();

        float s = acos(intersect.vec[1] / pointMaxDistance) / M_PI;
        float t = atan2(intersect.vec[2] / pointMaxDistance, intersect.vec[0] / pointMaxDistance) / (2 * M_PI) + 0.5;
        texCoords.push_back(Vector2(t, s));
    }
    
    // Send data to buffers
    loadGeometry();
}

void GeometryRender::handleNewTexture(unsigned char *data, int width, int height, int nrChannels) {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
// load and generate the texture
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
}

void GeometryRender::newShader() {
    loadGeometry();
}