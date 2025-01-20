#if defined(LINUX) || defined(MINGW)
    #include <SDL2/SDL.h>
#else
    #include <SDL.h>
#endif

// Third Party Libraries
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp> 

// C++ Standard Template Library (STL)
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>


// Our libraries
#include "Camera.hpp"
#include "Light.hpp"
#include "util.hpp"
#include "STL.hpp"

#include "globals.hpp"

/**
* Initialization of the graphics application.
*
* @return void
*/
void InitializeProgram(){
		// Initialize SDL
		if(SDL_Init(SDL_INIT_VIDEO)< 0){
				std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << "\n";
				exit(1);
		}
	
		// Setup the OpenGL Context
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
		// We want to request a double buffer for smooth updating.
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

		// Create an application window using OpenGL that supports SDL
		g.gGraphicsApplicationWindow = SDL_CreateWindow( "Lighting",
													SDL_WINDOWPOS_UNDEFINED,
													SDL_WINDOWPOS_UNDEFINED,
													g.gScreenWidth,
													g.gScreenHeight,
													SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );

		// Check if Window created.
		if( g.gGraphicsApplicationWindow == nullptr ){
				std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << "\n";
				exit(1);
		}

		// Create an OpenGL Graphics Context
		g.gOpenGLContext = SDL_GL_CreateContext( g.gGraphicsApplicationWindow );
		if( g.gOpenGLContext == nullptr){
				std::cout << "OpenGL context could not be created! SDL Error: " << SDL_GetError() << "\n";
				exit(1);
		}

		// Initialize GLAD Library
		if(!gladLoadGLLoader(SDL_GL_GetProcAddress)){
				std::cout << "glad did not initialize" << std::endl;
				exit(1);
		}

    // This was from the old STL example, we don't need it if we are using the Object class.
	// Setup Light(s)
	// g.gBunny = new STLFile;

    g.gLight.Initialize();
	// g.gBunny->Initialize();

	// Create and initialize the object
    g.gObject = new Object(g.objFilePath);
    g.gObject->Initialize();
}


/**
* PreDraw
* 
* @return void
*/
void PreDraw(){
	// Disable depth test and face culling.
    glEnable(GL_DEPTH_TEST);
		//glDepthFunc(GL_LESS);
    //glDisable(GL_CULL_FACE);

    // Set the polygon fill mode
    glPolygonMode(GL_FRONT_AND_BACK,g.gPolygonMode);

    // Initialize clear color
    glViewport(0, 0, g.gScreenWidth, g.gScreenHeight);
    glClearColor( 0.0f, 0.53f, 0.66f, 1.f );

    //Clear color buffer and Depth Buffer
  	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

/**
* Draw
* The render function gets called once per loop.
*
* @return void
*/
void Draw(){
	// Draw our Bunny
    // g.gBunny->PreDraw();
	// g.gBunny->Draw();
    
	// Draw our Object
    g.gObject->PreDraw();
    g.gObject->Draw();

    // Draw our light
    g.gLight.PreDraw();
    g.gLight.Draw();
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
    // Two static variables to hold the mouse position
    static int mouseX=g.gScreenWidth/2;
    static int mouseY=g.gScreenHeight/2; 

	SDL_Event e;
	while(SDL_PollEvent( &e ) != 0){
		if(e.type == SDL_QUIT){
			std::cout << "Goodbye! (Leaving MainApplicationLoop())" << std::endl;
			g.gQuit = true;
		}
        if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE){
			std::cout << "ESC: Goodbye! (Leaving MainApplicationLoop())" << std::endl;
            g.gQuit = true;
        }
        if(e.type==SDL_MOUSEMOTION){
            // Capture the change in the mouse position
            mouseX+=e.motion.xrel;
            mouseY+=e.motion.yrel;
            g.gCamera.MouseLook(mouseX,mouseY);
        }
	}

    // Retrieve keyboard state
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_UP]) {
        SDL_Delay(250);
        std::cout << "up:" << std::endl;
    }
    if (state[SDL_SCANCODE_DOWN]) {
        SDL_Delay(250); 
        std::cout << "Down:" << std::endl;
    }

    // Camera
    // Update our position of the camera
    if (state[SDL_SCANCODE_W]) {
        g.gCamera.MoveForward(0.1f);
    }
    if (state[SDL_SCANCODE_S]) {
        g.gCamera.MoveBackward(0.1f);
    }
    if (state[SDL_SCANCODE_A]) {
    }
    if (state[SDL_SCANCODE_D]) {
    }

    if (state[SDL_SCANCODE_1] || state[SDL_SCANCODE_TAB]) {
        SDL_Delay(250);
        if(g.gPolygonMode== GL_FILL){
            g.gPolygonMode = GL_LINE;
        }else{
            g.gPolygonMode = GL_FILL;
        }
    }
}

/**
* Main Application Loop
*
* @return void
*/
void MainLoop(){

    SDL_WarpMouseInWindow(g.gGraphicsApplicationWindow,g.gScreenWidth/2,g.gScreenHeight/2);
    SDL_SetRelativeMouseMode(SDL_TRUE);

	while(!g.gQuit){
		// Type of start of frame
		Uint32 start = SDL_GetTicks();

		Input();
		PreDraw();
		Draw();

		// Calculate how much time has elapsed
		Uint32 elapsedTime = SDL_GetTicks() - start;
		if(elapsedTime < 16){
			SDL_Delay(16-elapsedTime);
		}

		//Update screen of our specified window
		SDL_GL_SwapWindow(g.gGraphicsApplicationWindow);
	}
}

/**
* Destroy any global objects.
*
* @return void
*/
void CleanUp(){
		//Destroy SDL2 Window
		SDL_DestroyWindow(g.gGraphicsApplicationWindow );
		g.gGraphicsApplicationWindow = nullptr;

		//Quit SDL subsystems
		SDL_Quit();
}

// Function to parse command-line arguments
void ParseArguments(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: ./prog <path_to_obj_file>\n";
        exit(EXIT_FAILURE);
    }
    g.objFilePath = argv[1]; // Store the file path in a global variable
}

/**
* The entry point.
*
* @return program status
*/
int main( int argc, char* argv[] ){
    std::cout << "Use w and s keys to move forward and back\n";
    std::cout << "Use mouse to look around\n";
    std::cout << "Use 1 to toggle wireframe\n";
    std::cout << "Press ESC to quit\n";

	// Parse command-line arguments
    ParseArguments(argc, argv);

	// 1. Setup the graphics program
	InitializeProgram();
		
	// 2. Call the main application loop
	MainLoop();	

	// 3. Call the cleanup function when our program terminates
	CleanUp();

	return 0;
}