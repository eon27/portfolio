/*
 *  Workshop 1
 *  Computer Graphics course
 *  Dept Computing Science, Umea University
 *  Stefan Johansson, stefanj@cs.umu.se
 */

#include "openglwindow.h"
#include "lib/objLoader/objLoader.h"
#include "lib/ImGui/imgui.h"
#include "lib/ImGui/imgui_impl_glfw.h"
#include "lib/ImGui/imgui_impl_opengl3.h"
#include "lib/ImGuiFileDialog/ImGuiFileDialog.h"

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image/stb_image.h"

using namespace std;

// Initialize OpenGL context and viewport.
OpenGLWindow::OpenGLWindow(string title, int width, int height)
{
    // Initialize glfw
    if (!glfwInit())
        exit(EXIT_FAILURE);

    // Set minimum supported OpenGL version and OpenGL profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create OpenGL window
    windowWidth = width;
    windowHeight = height;
    glfwWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (glfwWindow == nullptr) {
        glfwTerminate();
        cerr << "Could not open window or initialize OpenGL context." << endl;
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(glfwWindow);
    glfwSwapInterval(1); 
    
    // Initialize glew
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        cout << "glew init error:" << endl << glewGetErrorString(glewError) << endl;
        exit(EXIT_FAILURE);
    }

    if ( !GLEW_VERSION_4_3 ) {
        cout << "Warning: OpenGL 4.3+ not supported by the GPU!" << endl;
        cout << "Decreace supported OpenGL version if needed." << endl;
    }

    // Set graphics attributes
    glPointSize(5.0); // Unsupported in OpenGL ES 2.0
    glLineWidth(1.0);
    glClearColor(0.2, 0.2, 0.2, 0.0);

    glViewport(0, 0, width, height);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // Enable Gamepad Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
    ImGui_ImplOpenGL3_Init(NULL);
}

OpenGLWindow::~OpenGLWindow()
{   
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(glfwWindow);
    glfwTerminate();
}

// OpenGL error handler
bool 
OpenGLWindow::checkOpenGLError() const
{
    bool foundError = false;
    GLenum glError = glGetError();
    
    while (glError != GL_NO_ERROR) {
        cerr << "glError: " << glError << endl;
        foundError = true;
        glError = glGetError();
    }
    return foundError;
}

int 
OpenGLWindow::width() const
{
    return windowWidth;
}

int 
OpenGLWindow::height() const
{
    return windowHeight;
}

GLFWwindow* 
OpenGLWindow::window() const
{
    return glfwWindow;
}

// Read shader source files
string
OpenGLWindow::readShaderSource(const string shaderFile) const
{
    string shaderSource;
    string line;

    fstream fs(shaderFile, ios::in);
    if(!fs)
        return shaderSource;

    while (!fs.eof()) {
        getline(fs, line);
        shaderSource.append(line + '\n');
    }
    fs.close();
    return shaderSource;
}


// Initialize OpenGL shader program
GLuint 
OpenGLWindow::initProgram(const string vShaderFile, const string fShaderFile) const
{
    GLuint program;
    int i;
    GLint  linked;

    struct shaders_t{
        string   filename;
        GLenum   type;
    };

    shaders_t shaders[2] = {
        { vShaderFile, GL_VERTEX_SHADER },
        { fShaderFile, GL_FRAGMENT_SHADER }
    };

    program = glCreateProgram();
    for (i = 0; i < 2; ++i ) {
        GLuint shader;
        GLint  compiled;

        string shaderSource = readShaderSource( shaders[i].filename );
        if ( shaderSource.empty() ) {
            cerr << "Failed to read " << shaders[i].filename << endl;
            exit( EXIT_FAILURE );
        }

        shader = glCreateShader( shaders[i].type );
        const char *shaderSrc = shaderSource.c_str();
        glShaderSource( shader, 1, &shaderSrc, NULL );
        glCompileShader( shader );
        checkOpenGLError();

        glGetShaderiv( shader, GL_COMPILE_STATUS, &compiled );
        if ( !compiled ) {
            GLint  logSize;

            cerr << "Failed to compile " << shaders[i].filename << endl;
            glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &logSize );
            if (logSize > 0) {
                char logMsg[logSize+1];
                glGetShaderInfoLog( shader, logSize, nullptr, &(logMsg[0]) );
                cerr << "Shader info log: " << logMsg << endl;
            }
            exit( EXIT_FAILURE );
        }
        glAttachShader( program, shader );
    }

    /* link  and error check */
    glLinkProgram(program);
    checkOpenGLError();

    glGetProgramiv( program, GL_LINK_STATUS, &linked );
    if ( !linked ) {
        GLint  logSize;

        cerr << "Shader program failed to link!" << endl;

        glGetProgramiv( program, GL_INFO_LOG_LENGTH, &logSize);
        if ( logSize > 0 ) {
            char logMsg[logSize+1];
            glGetProgramInfoLog( program, logSize, NULL, &(logMsg[0]) );
            cerr << "Program info log: " << logMsg << endl;
        }
        exit( EXIT_FAILURE );
    }

    return program;
}


// The window resize callback function
void 
OpenGLWindow::resizeCallback(GLFWwindow* window, int width, int height)
{
    reshape(width, height);
    windowHeight = height;
    windowWidth = width;
}

void OpenGLWindow::reshape(const int width, const int height) const {
    if (glfwGetCurrentContext() == nullptr) return;
    glViewport(0,0,width, height);
}

// GLFW error callback function
void 
OpenGLWindow::errorCallback(int error, const char* description)
{
    cerr << "GLFW error: " << description << endl;
}

void
OpenGLWindow::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    else if (action != GLFW_RELEASE) { // Capture GLFW_PRESS and GLFW_RELEASE
        controlls(key);
    }
}

// Handling input with the gui as recomended by https://github.com/ocornut/imgui/blob/master/docs/FAQ.md#q-how-can-i-tell-whether-to-dispatch-mousekeyboard-to-dear-imgui-or-my-application
void
OpenGLWindow::mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent(xpos, ypos);
    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
        // Flipping delta Y because positiv Y is downwards on screen.
        rotateCamera(mousePosX-xpos, ypos-mousePosY);
        mousePosX = xpos;
        mousePosY = ypos;
    }
}

void
OpenGLWindow::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    ImGuiIO& io = ImGui::GetIO();
    io.AddMouseButtonEvent(button, action);
    if (!io.WantCaptureMouse) {
        if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            if (action == GLFW_PRESS) {
                glfwGetCursorPos(window, &mousePosX, &mousePosY);
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            else if (action == GLFW_RELEASE) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

void
OpenGLWindow::DrawGui()
{
    IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing dear imgui context.");

    static ImGuiSliderFlags flags = ImGuiSliderFlags_AlwaysClamp;
    static ImGuiFileDialog fileDialog;    
    static ImGuiFileDialog textureDialog; // New dialog for textures
    
    ImGui::Begin("3D Studio");


    if (ImGui::CollapsingHeader("OBJ File")) {
        ImGui::Text("OBJ file: %s", objFileName.c_str());
        if (ImGui::Button("Open File"))
            fileDialog.OpenDialog("ChooseFileDlgKey", "Choose File", ".obj", ".");
        
        if (fileDialog.Display("ChooseFileDlgKey")) {
            if (fileDialog.IsOk() == true) {
                objFileName = fileDialog.GetCurrentFileName();
                objFilePath = fileDialog.GetCurrentPath();
                cout << "OBJ file: " << objFileName << endl << "Path: " << objFilePath << endl;
                openNewObject(objFilePath + '/' + objFileName);
            } else {
                // Return a message to the user if the file could not be opened
            }
            fileDialog.Close();
        }
    }
    
    if (ImGui::CollapsingHeader("Light")) {
        ImGui::Text("Light source position");
        ImGui::PushItemWidth(100);
        ImGui::InputFloat("x", &lightPos[0], 0.5f, 1.0f, "%1.1f"); ImGui::SameLine();
        ImGui::InputFloat("y", &lightPos[1], 0.5f, 1.0f, "%1.1f"); ImGui::SameLine();
        ImGui::InputFloat("z", &lightPos[2], 0.5f, 1.0f, "%1.1f");
        ImGui::PopItemWidth();

        ImGui::Text("Light source intensity:");
        ImGui::ColorEdit3("Light", lightColor);
        
        ImGui::Text("Ambient light intensity:");
        ImGui::ColorEdit3("Ambient", ambientColor);
    }
    
    if (ImGui::CollapsingHeader("Object Material")) {
        ImGui::Text("Ambient coefficient:");
        ImGui::ColorEdit3("Ambient color", materialAmbient);
        
        ImGui::Text("Diffuse coefficient:");
        ImGui::ColorEdit3("Diffuse color", materialDiffuse);
        
        ImGui::Text("Specular coefficient:");
        ImGui::ColorEdit3("Specular color", materialSpecular);

        ImGui::SliderFloat("Shininess", &materialShininess, 1.0f, 1000.0f, "%1.0f", flags);
    }
    
    if (ImGui::CollapsingHeader("Object Texture")) {
        ImGui::Checkbox("Show texture", &textureShow);
        ImGui::Text("Texture file: %s", textureFileName.c_str());
        if (ImGui::Button("Open Texture File"))
            textureDialog.OpenDialog("ChooseFileDlgKey", "Choose Texture File",
            	                     ".jpg,.bmp,.dds,.hdr,.pic,.png,.psd,.tga", ".");
        
        if (textureDialog.Display("ChooseFileDlgKey")) {
            if (textureDialog.IsOk() == true) {
                textureFileName = textureDialog.GetCurrentFileName();
                textureFilePath = textureDialog.GetCurrentPath();
                cout << "Texture file: " << textureFileName << endl << "Path: " << textureFilePath << endl;
                openNewTexture(textureFilePath + '/' + textureFileName);
            } else {
                // Return a message to the user if the file could not be opened
            }
            // close
            textureDialog.Close();
        }
    }
      
    if (ImGui::CollapsingHeader("Projection")) {
        const char* items[] = {"Perspective", "Parallel" };
        if (ImGui::Combo("projektion", &proj_current_idx, items, IM_ARRAYSIZE(items), IM_ARRAYSIZE(items)));
        if (proj_current_idx == 0) {
            ImGui::SliderFloat("Field of view",&fov, 20.0f, 160.0f, "%1.0f", flags);
            ImGui::SliderFloat("Far",&farplane, 1.0f, 1000.0f, "%1.0f", flags);
        }
        if (proj_current_idx == 1) {
            ImGui::SliderFloat("Top",&top, 1.0f, 100.0f, "%.1f", flags);
            ImGui::SliderFloat("Far",&farplane, 1.0f, 1000.0f, "%1.0f", flags);
            ImGui::SliderFloat("Oblique scale",&obliqueScale, 0.0f, 1.0f, "%.1f", flags);
            ImGui::SliderAngle("Oblique angle",&obliqueAngleRad, 15, 75, "%1.0f", flags);
        }
    }

    if (ImGui::CollapsingHeader("Shading")) {
        const char* items[] = {"Phong", "Gouraud", "Wireframe"};
        int shading_previous_idx = shading_current_idx;
        ImGui::Combo("shading", &shading_current_idx, items, IM_ARRAYSIZE(items), IM_ARRAYSIZE(items));
        if (shading_previous_idx != shading_current_idx) {
            newShader();
        }
    }

    ImGui::End();
}

// Start the GLFW loop
void 
OpenGLWindow::start()
{
    // Loop until the user closes the window
    while (!glfwWindowShouldClose(glfwWindow)) {

        // Poll movement input
        pollInput();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Draw the gui
        DrawGui();

        // Call display in geomentryRender to render the scene
        display();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        // Swap buffers
        glfwSwapBuffers(glfwWindow);

        // Sleep and wait for an event
        glfwPollEvents();
    }
    
}

void OpenGLWindow::pollInput() {
    auto timeNow = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float>(timeNow - time).count();
    time = timeNow;

    if (glfwGetKey(glfwWindow, GLFW_KEY_W) == GLFW_PRESS) moveCamera(GLFW_KEY_W, deltaTime);
    if (glfwGetKey(glfwWindow, GLFW_KEY_A) == GLFW_PRESS) moveCamera(GLFW_KEY_A, deltaTime);
    if (glfwGetKey(glfwWindow, GLFW_KEY_S) == GLFW_PRESS) moveCamera(GLFW_KEY_S, deltaTime);
    if (glfwGetKey(glfwWindow, GLFW_KEY_D) == GLFW_PRESS) moveCamera(GLFW_KEY_D, deltaTime);
    if (glfwGetKey(glfwWindow, GLFW_KEY_E) == GLFW_PRESS) moveCamera(GLFW_KEY_E, deltaTime);
    if (glfwGetKey(glfwWindow, GLFW_KEY_Q) == GLFW_PRESS) moveCamera(GLFW_KEY_Q, deltaTime);
}

// Render the scene 
void OpenGLWindow::displayNow()
{
    if (glfwGetCurrentContext() == nullptr)
        return;

    display();
}

void OpenGLWindow::openNewObject(string filename) {
    // Make a pointer to the string because objLoader wants a char*
    char* namePointer;
    namePointer = &filename[0];

    int status = objData.load(namePointer); // Returns 0 on error.
    if (status == 0) {
        return;
    }
    handleNewObject();
}

void OpenGLWindow::openNewTexture(string filename) {
    // Make a pointer to the string because stb_image wants a char*
    char* namePointer;
    namePointer = &filename[0];
    
    int width, height, nrChannels;
    unsigned char *data = stbi_load(namePointer, &width, &height, &nrChannels, 0);
    
    handleNewTexture(data, width, height, nrChannels);
    stbi_image_free(data);
}