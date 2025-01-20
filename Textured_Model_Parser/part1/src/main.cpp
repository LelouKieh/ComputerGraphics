#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp> 

// C++ Standard Template Library (STL)
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

// Our libraries
#include "Camera.hpp"
#include "Texture.hpp"
#include "Object.hpp"

#include "globals.hpp"

// OpenGL Objects
// Vertex Array Object (VAO)
GLuint gVertexArrayObject					= 0;
// Vertex Buffer Object (VBO)
GLuint 	gVertexBufferObject					= 0;
// Index Buffer Object (IBO)
GLuint 	gIndexBufferObject                  = 0;


// Error Handling Routines
static void GLClearAllErrors(){
    while(glGetError() != GL_NO_ERROR){
    }
}

// Returns true if we have an error
static bool GLCheckErrorStatus(const char* function, int line){
    while(GLenum error = glGetError()){
        std::cout << "OpenGL Error:" << error 
                  << "\tLine: " << line 
                  << "\tfunction: " << function << std::endl;
        return true;
    }
    return false;
}

#define GLCheck(x) GLClearAllErrors(); x; GLCheckErrorStatus(#x,__LINE__);



/**
* LoadShaderAsString takes a filepath as an argument and will read line by line a file and return a string that is meant to be compiled at runtime for a vertex, fragment, geometry, tesselation, or compute shader.
* e.g.
*       LoadShaderAsString("./shaders/filepath");
*
* @param filename Path to the shader file
* @return Entire file stored as a single string 
*/
std::string LoadShaderAsString(const std::string& filename){
    // Resulting shader program loaded as a single string
    std::string result = "";

    std::string line = "";
    std::ifstream myFile(filename.c_str());

    if(myFile.is_open()){
        while(std::getline(myFile, line)){
            result += line + '\n';
        }
        myFile.close();

    }

    return result;
}


/**
* CompileShader will compile any valid vertex, fragment, geometry, tesselation, or compute shader.
* e.g.
*	    Compile a vertex shader: 	CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
*       Compile a fragment shader: 	CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
*
* @param type We use the 'type' field to determine which shader we are going to compile.
* @param source : The shader source code.
* @return id of the shaderObject
*/
GLuint CompileShader(GLuint type, const std::string& source){
	// Compile shaders
	GLuint shaderObject;

	if(type == GL_VERTEX_SHADER){
		shaderObject = glCreateShader(GL_VERTEX_SHADER);
	}else if(type == GL_FRAGMENT_SHADER){
		shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	}

	const char* src = source.c_str();
	// The source of shader
	glShaderSource(shaderObject, 1, &src, nullptr);
	// compile shader
	glCompileShader(shaderObject);

	// Retrieve the result of compilation
	int result;
	// retrieve the compilation status
	glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &result);

	if(result == GL_FALSE){
		int length;
		glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
		char* errorMessages = new char[length]; // Could also use alloca here.
		glGetShaderInfoLog(shaderObject, length, &length, errorMessages);

		if(type == GL_VERTEX_SHADER){
			std::cout << "ERROR: GL_VERTEX_SHADER compilation failed!\n" << errorMessages << "\n";
		}else if(type == GL_FRAGMENT_SHADER){
			std::cout << "ERROR: GL_FRAGMENT_SHADER compilation failed!\n" << errorMessages << "\n";
		}
		// Reclaim memory
		delete[] errorMessages;

		// Delete shader
		glDeleteShader(shaderObject);

		return 0;
	}

  return shaderObject;
}



/**
* Creates a graphics program object (i.e. graphics pipeline) with a Vertex Shader and a Fragment Shader
*
* @param vertexShaderSource Vertex source code as a string
* @param fragmentShaderSource Fragment shader source code as a string
* @return id of the program Object
*/
GLuint CreateShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource){

    // Create a new program object
    GLuint programObject = glCreateProgram();

    // Compile shaders
    GLuint myVertexShader   = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint myFragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    if (myVertexShader == 0 || myFragmentShader == 0) {
        std::cerr << "Shader compilation failed. Cannot create shader program." << std::endl;
        return 0;
    }
	
	// Link two shader programs together.
    glAttachShader(programObject,myVertexShader);
    glAttachShader(programObject,myFragmentShader);
    glLinkProgram(programObject);

    // Validate program
    glValidateProgram(programObject);

    glDetachShader(programObject,myVertexShader);
    glDetachShader(programObject,myFragmentShader);
    glDeleteShader(myVertexShader);
    glDeleteShader(myFragmentShader);

    return programObject;
}


/**
* Initialization of the graphics application.
*
* @return void
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
}


/**
* PreDraw
* @return void
*/
void PreDraw(){
    // Disable depth test and face culling.
    // glDisable(GL_DEPTH_TEST);
    // glDisable(GL_CULL_FACE);

    // Enable texture mapping
    glEnable(GL_TEXTURE_2D);

    // Initialize clear color
    glViewport(0, 0, g.gScreenWidth, g.gScreenHeight);
    glClearColor(1.f, 1.f, 0.f, 1.f);

    // Clear color buffer and Depth Buffer
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    g.gObject->PreDraw();
}



/**
* Draw
* The render function gets called once per loop.
*
* @return void
*/
void Draw(){
  // Enable attributes
	glBindVertexArray(gVertexArrayObject);

	// Select the vertex buffer object
  glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);

  //Render data
  glDrawElements(GL_TRIANGLES,
                    6,
                    GL_UNSIGNED_INT,
                    0);

  glUseProgram(0);
}


/**
* Helper Function to get OpenGL Version Information
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
* Function called in the Main application loop to handle user input
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
        if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE){
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
    if (state[SDL_SCANCODE_W]) {
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
    if (state[SDL_SCANCODE_TAB]) {
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
* Main Application Loop
*
* @return void
*/
void MainLoop(){
    // Center the mouse in the window
    SDL_WarpMouseInWindow(g.gGraphicsApplicationWindow, g.gScreenWidth / 2, g.gScreenHeight / 2);

    // While application is running
    while (!g.gQuit) {
        // Handle Input
        Input();

        // Pre-draw setup
        PreDraw();

        // Draw the scene
        // Draw();
		g.gObject->Draw();

        // Update screen
        SDL_GL_SwapWindow(g.gGraphicsApplicationWindow);
    }
}



/**
* Destroy any global objects.
*
* @return void
*/
void CleanUp(){
    // Destroy SDL window
    SDL_DestroyWindow(g.gGraphicsApplicationWindow);
    g.gGraphicsApplicationWindow = nullptr;

    // Delete OpenGL objects
    glDeleteBuffers(1, &gVertexBufferObject);
    glDeleteVertexArrays(1, &gVertexArrayObject);

    // Delete shader program
    glDeleteProgram(g.gGraphicsPipelineShaderProgram);
   
    // Delete the Object
    delete g.gObject;
    g.gObject = nullptr;
    
    // Quit SDL
    SDL_Quit();
}


/**
* The entry point.
*
* @return program status
*/
int main( int argc, char* args[] ){
    std::cout << "Use arrow keys to move and rotate\n";
    std::cout << "Use WASD to move\n";

    if (argc < 2) {
        std::cout << "Usage: " << args[0] << " path/to/objfile.obj\n";
        return 1;
    }

    g.objFilePath = args[1];

    // Initialize program
    InitializeProgram();

    // Create and initialize object
    g.gObject = new Object(g.objFilePath);
    g.gObject->Initialize();

    // Main loop
    MainLoop();

    // Clean up
    CleanUp();

    return 0;
}
