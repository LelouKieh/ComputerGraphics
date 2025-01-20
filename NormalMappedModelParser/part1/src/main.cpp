#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>

// C++ Standard Template Library (STL)
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

// Our libraries
#include "Camera.hpp"
#include "Texture.hpp"
#include "Object.hpp"
#include "util.hpp"

#include "globals.hpp"

// OpenGL Objects
// Vertex Array Object (VAO)
GLuint gVertexArrayObject					= 0;
// Vertex Buffer Object (VBO)
GLuint 	gVertexBufferObject					= 0;
// Index Buffer Object (IBO)
GLuint 	gIndexBufferObject                  = 0;

/**
 * @brief Sets up the SDL environment and initializes OpenGL context and window.
 * 
 * This function initializes the SDL library, sets OpenGL context attributes,
 * and creates the application window. It also initializes the GLAD library 
 * to handle OpenGL function pointers.
 * 
 * @throws runtime_error if SDL or GLAD initialization fails or if the window cannot be created.
 */
void InitializeProgram(){
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << "\n";
        exit(1);
    }

    // Setup the OpenGL Context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // Create an application window using OpenGL that supports SDL
    g.gGraphicsApplicationWindow = SDL_CreateWindow("OpenGL First Program",
                                                    SDL_WINDOWPOS_UNDEFINED,
                                                    SDL_WINDOWPOS_UNDEFINED,
                                                    g.gScreenWidth,
                                                    g.gScreenHeight,
                                                    SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    // Check if Window created.
    if (g.gGraphicsApplicationWindow == nullptr) {
        std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << "\n";
        exit(1);
    }

    // Create an OpenGL Graphics Context
    g.gOpenGLContext = SDL_GL_CreateContext(g.gGraphicsApplicationWindow);
    if (g.gOpenGLContext == nullptr) {
        std::cout << "OpenGL context could not be created! SDL Error: " << SDL_GetError() << "\n";
        exit(1);
    }

    // Initialize GLAD Library
    if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
        std::cout << "glad did not initialize" << std::endl;
        exit(1);
    }
    g.gLight.Initialize();
}


/**
 * @brief Sets OpenGL states and clears buffers before drawing the frame.
 *
 * This function configures OpenGL settings needed for the frame, such as
 * enabling texture mapping, setting viewport dimensions, clearing the color 
 * and depth buffers, and preparing the object for drawing if it exists.
 *
 * @return void
 */
void PreDraw(){
    // Enable texture mapping
    glEnable(GL_TEXTURE_2D);

    // Initialize clear color
    glViewport(0, 0, g.gScreenWidth, g.gScreenHeight);
    glClearColor(1.f, 1.f, 0.f, 1.f);

    // Clear color buffer and Depth Buffer
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    if (g.gObject != nullptr) {
        g.gObject->PreDraw();
    }
}


/**
 * @brief Specifies and initializes vertex data for rendering, including VAO, VBO, and IBO.
 *
 * This function loads texture files and shaders, sets up vertex attribute data
 * (positions, normals, texture coordinates, tangents, and bitangents), and
 * defines index buffer data to use for indexed drawing. It also binds 
 * and configures VAO, VBO, and IBO to prepare them for rendering.
 * 
 * This function is for the default brick with texture and normal map only.
 *
 * @return void
 */
void VertexSpecification(){
	// We will load a texture here prior
	g.gTexture.LoadTexture("./starter/brick.ppm");
    g.gNormalMap.LoadTexture("./starter/normal.ppm");
    std::string brickVertexShader = LoadShaderAsString("./shaders/brick_vert.glsl");
    std::string brickFragmentShader = LoadShaderAsString("./shaders/brick_frag.glsl");
    g.gGraphicsPipelineShaderProgram = CreateShaderProgram(brickVertexShader, brickFragmentShader);

    // Geometry Data: Positions, Normals, Texture Coordinates, Tangents, Bitangents
    const std::vector<GLfloat> vertexData = {
        // Positions          // Normals        // TexCoords  // Tangents       // Bitangents
        // Vertex 0
        -1.0f, -1.0f, 0.0f,   // Position
         0.0f, 0.0f, 1.0f,    // Normal
         0.0f, 0.0f,          // TexCoords
         1.0f, 0.0f, 0.0f,    // Tangent
         0.0f, 1.0f, 0.0f,    // Bitangent
        // Vertex 1
         1.0f, -1.0f, 0.0f,
         0.0f, 0.0f, 1.0f,
         1.0f, 0.0f,
         1.0f, 0.0f, 0.0f,
         0.0f, 1.0f, 0.0f,
        // Vertex 2
        -1.0f,  1.0f, 0.0f,
         0.0f, 0.0f, 1.0f,
         0.0f, 1.0f,
         1.0f, 0.0f, 0.0f,
         0.0f, 1.0f, 0.0f,
        // Vertex 3
         1.0f,  1.0f, 0.0f,
         0.0f, 0.0f, 1.0f,
         1.0f, 1.0f,
         1.0f, 0.0f, 0.0f,
         0.0f, 1.0f, 0.0f,
    };
    // Index buffer data for a quad
    const std::vector<GLuint> indexBufferData = {0, 1, 2, 2, 1, 3};

    // Setup VAO, VBO, IBO
    glGenVertexArrays(1, &gVertexArrayObject);
    glBindVertexArray(gVertexArrayObject);

    glGenBuffers(1, &gVertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &gIndexBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferData.size() * sizeof(GLuint), indexBufferData.data(), GL_STATIC_DRAW);

    // Define vertex attribute pointers
    GLsizei stride = 14 * sizeof(GLfloat);

    // Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)(0));

    // Normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));

    // Texture Coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(GLfloat)));

    // Tangents
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(8 * sizeof(GLfloat)));

    // Bitangents
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride, (void*)(11 * sizeof(GLfloat)));

    glBindVertexArray(0);
	// Disable any attributes we opened in our Vertex Attribute Arrray,
	// as we do not want to leave them open. 
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
}


/**
 * @brief Issues draw calls for rendering the scene.
 *
 * This function sets shader program uniforms (model, view, and projection matrices),
 * binds textures, and issues the draw call to render the object. 
 * Light and camera positions are also set as uniforms for shading.
 *
 * @return void
 */
void Draw(){
    glUseProgram(g.gGraphicsPipelineShaderProgram);

    // Set transformation matrices
    GLint modelLoc = glGetUniformLocation(g.gGraphicsPipelineShaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(g.gGraphicsPipelineShaderProgram, "view");
    GLint projLoc = glGetUniformLocation(g.gGraphicsPipelineShaderProgram, "projection");

    glm::mat4 model = glm::mat4(1.0f); // Modify as needed
    glm::mat4 view = g.gCamera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)g.gScreenWidth / (float)g.gScreenHeight, 0.1f, 100.0f);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Set light and view positions
    GLint lightPosLoc = glGetUniformLocation(g.gGraphicsPipelineShaderProgram, "lightPos");
    GLint viewPosLoc = glGetUniformLocation(g.gGraphicsPipelineShaderProgram, "viewPos");

    glUniform3fv(lightPosLoc, 1, glm::value_ptr(g.gLight.GetPosition()));
    glUniform3fv(viewPosLoc, 1, glm::value_ptr(g.gCamera.GetPosition()));

    // Bind textures
    glActiveTexture(GL_TEXTURE0);
    g.gTexture.Bind(0);
    glUniform1i(glGetUniformLocation(g.gGraphicsPipelineShaderProgram, "diffuseMap"), 0);

    glActiveTexture(GL_TEXTURE1);
    g.gNormalMap.Bind(1);
    glUniform1i(glGetUniformLocation(g.gGraphicsPipelineShaderProgram, "normalMap"), 1);

    // Bind VAO and draw
    glBindVertexArray(gVertexArrayObject);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glUseProgram(0);
}


/**
 * @brief Prints the current OpenGL version and graphics driver information.
 *
 * Retrieves and displays details about the OpenGL implementation, including 
 * the vendor, renderer, OpenGL version, and shading language version.
 *
 * @return void
 */
void getOpenGLVersionInfo(){
  std::cout << "Vendor: " << glGetString(GL_VENDOR) << "\n";
  std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
  std::cout << "Version: " << glGetString(GL_VERSION) << "\n";
  std::cout << "Shading language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
}


/**
 * @brief Processes SDL input events for controlling the application.
 *
 * This function checks for SDL events such as window close, specific key presses, 
 * and keyboard states. It allows the user to move the camera, toggle wireframe 
 * mode, and interact with the object through keyboard input.
 *
 * @return void
 */
void Input(){
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            std::cout << "Goodbye! (Leaving MainApplicationLoop())" << std::endl;
            g.gQuit = true;
        }
        if(e.type == SDL_KEYDOWN && (e.key.keysym.sym == SDLK_ESCAPE || e.key.keysym.sym == SDLK_q)){
            std::cout << "ESC: Goodbye! (Leaving MainApplicationLoop())" << std::endl;
            g.gQuit = true;
        }
    }

    const Uint8* state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_UP]) {
        g.g_uOffset += 0.01f;
        std::cout << "g_uOffset: " << g.g_uOffset << std::endl;
    }
    if (state[SDL_SCANCODE_DOWN]) {
        g.g_uOffset -= 0.01f;
        std::cout << "g_uOffset: " << g.g_uOffset << std::endl;
    }
    if (state[SDL_SCANCODE_LEFT]) {
        g.g_uRotate -= 1.0f;
        std::cout << "g_uRotate: " << g.g_uRotate << std::endl;
    }
    if (state[SDL_SCANCODE_RIGHT]) {
        g.g_uRotate += 1.0f;
        std::cout << "g_uRotate: " << g.g_uRotate << std::endl;
    }
    if (state[SDL_SCANCODE_J]) {
        g.gCamera.MoveUp(0.01f);
    }
    if (state[SDL_SCANCODE_K]) {
        g.gCamera.MoveDown(0.01f);
    }

    // Camera movement
    if (state[SDL_SCANCODE_E]) {
        g.gCamera.MoveForward(0.01f);
    }
    if (state[SDL_SCANCODE_S]) {
        g.gCamera.MoveBackward(0.01f);
    }
    if (state[SDL_SCANCODE_A]) {
        g.gCamera.MoveLeft(0.01f);
    }
    if (state[SDL_SCANCODE_D]) {
        g.gCamera.MoveRight(0.01f);
    }
    if (state[SDL_SCANCODE_TAB] || state[SDL_SCANCODE_W]) {
        SDL_Delay(250);
        g.gWireframeMode = !g.gWireframeMode;
        if (g.gWireframeMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); 
        }
    }

    // Mouse look
    int mouseX, mouseY;
    SDL_GetGlobalMouseState(&mouseX, &mouseY);
    g.gCamera.MouseLook(mouseX, mouseY);
}


/**
 * @brief The main application loop handling input, rendering, and screen updates.
 *
 * This loop continues running until a quit event is received. For each iteration,
 * it processes user input, prepares OpenGL states, calls the draw functions 
 * for objects and lights, and updates the display.
 *
 * @return void
 */
void MainLoop(){
    SDL_WarpMouseInWindow(g.gGraphicsApplicationWindow, g.gScreenWidth / 2, g.gScreenHeight / 2);

    // While application is running
    while (!g.gQuit) {
        // Handle Input
        Input();

        // Pre-draw setup
        PreDraw();

        // Draw the scene
        if (g.gObject != nullptr) {
            g.gObject->Draw();
        } else {
            Draw();
        }

        g.gLight.PreDraw();
        g.gLight.Draw();

        // Update screen
        SDL_GL_SwapWindow(g.gGraphicsApplicationWindow);
    }
}


/**
 * @brief Cleans up and deallocates resources before application termination.
 *
 * This function deletes the OpenGL objects (VBOs, VAOs, shaders) and SDL window 
 * if they were created. It also releases dynamically allocated memory, ensuring 
 * that no resources are left unfreed at program exit.
 *
 * @return void
 */
void CleanUp(){
    SDL_DestroyWindow(g.gGraphicsApplicationWindow);
    g.gGraphicsApplicationWindow = nullptr;

    // Delete OpenGL objects if they were created
    if (gVertexBufferObject) glDeleteBuffers(1, &gVertexBufferObject);
    if (gIndexBufferObject) glDeleteBuffers(1, &gIndexBufferObject);
    if (gVertexArrayObject) glDeleteVertexArrays(1, &gVertexArrayObject);

    // Delete shader program
    if (g.gGraphicsPipelineShaderProgram) glDeleteProgram(g.gGraphicsPipelineShaderProgram);

    // Delete the Object if it exists
    if (g.gObject) {
        delete g.gObject;
        g.gObject = nullptr;
    }

    // Quit SDL
    SDL_Quit();
}


/**
* The entry point into our C++ programs.
*
* @return program status
*/
int main( int argc, char* args[] ){
    std::cout << "Use arrow keys to move and rotate\n";
    std::cout << "Use WASD to move\n";

    // Initialize program
    InitializeProgram();

    if (argc < 2) {
        std::cout << "No OBJ file specified, using default square.\n";
        VertexSpecification();
        // No OBJ file, so we don't create g.gObject
        g.gObject = nullptr;
    } else {
        // We have an OBJ file specified
        g.objFilePath = args[1];

        // Create and initialize object
        g.gObject = new Object(g.objFilePath);
        g.gObject->Initialize();
    }

    // Main loop
    MainLoop();

    // Clean up
    CleanUp();

    return 0;
}
