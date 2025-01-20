// Third Party Libraries
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

#include "Camera.hpp"


// Screen Dimensions
int gScreenWidth 						= 640;
int gScreenHeight 						= 480;
SDL_Window* gGraphicsApplicationWindow 	= nullptr;
SDL_GLContext gOpenGLContext			= nullptr;

// Main loop flag
bool gQuit = false;

// shader
GLuint gGraphicsPipelineShaderProgram	= 0;

// OpenGL Objects
// Vertex Array Object (VAO)
GLuint gVertexArrayObject					= 0;
GLuint gVertexArrayObjectFloor= 0;
// Vertex Buffer Object (VBO)
GLuint 	gVertexBufferObject					= 0;
GLuint  gVertexBufferObjectFloor            = 0;
// Index Buffer Object (IBO)
GLuint 	gIndexBufferObject                  = 0;
GLuint 	gIndexBufferObjectFloor             = 0;

bool  g_rotatePositive=true;
float g_uRotate=0.0f;

// Camera
Camera gCamera;
const float CAMERA_SPEED = 0.002f; // control the speed of camera movement
const float MOUSE_SENSITIVITY = 0.2f; // control the sensitivity of the mouse
const float ROTATION_SPEED_UPDATE = 0.01f;



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
* @param type determine which shader we are going to compile.
* @param source : The shader source code.
* @return id of the shaderObject
*/
GLuint CompileShader(GLuint type, const std::string& source){
	// Compile our shaders
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
		char* errorMessages = new char[length];
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
* Create the graphics pipeline
*
* @return void
*/
void CreateGraphicsPipeline(){

    std::string vertexShaderSource      = LoadShaderAsString("./shaders/vert.glsl");
    std::string fragmentShaderSource    = LoadShaderAsString("./shaders/frag.glsl");

	gGraphicsPipelineShaderProgram = CreateShaderProgram(vertexShaderSource,fragmentShaderSource);
}


/**
* Initialization of the graphics application. Typically this will involve setting up a window
* and the OpenGL Context (with the appropriate version)
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
	// Use OpenGL 4.1 core or greater
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
	// We want to request a double buffer for smooth updating.
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	// Create an application window using OpenGL that supports SDL
	gGraphicsApplicationWindow = SDL_CreateWindow( "OpenGL First Program",
													SDL_WINDOWPOS_UNDEFINED,
													SDL_WINDOWPOS_UNDEFINED,
													gScreenWidth,
													gScreenHeight,
													SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );

	// Check if Window did not create.
	if( gGraphicsApplicationWindow == nullptr ){
		std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << "\n";
		exit(1);
	}

	// Create an OpenGL Graphics Context
	gOpenGLContext = SDL_GL_CreateContext( gGraphicsApplicationWindow );
	if( gOpenGLContext == nullptr){
		std::cout << "OpenGL context could not be created! SDL Error: " << SDL_GetError() << "\n";
		exit(1);
	}

	// Initialize GLAD Library
	if(!gladLoadGLLoader(SDL_GL_GetProcAddress)){
		std::cout << "glad did not initialize" << std::endl;
		exit(1);
	}
	
}

/**
* Setup geometry during the vertex specification step
*
* @return void
*/
void VertexSpecification(){

	// Geometry Data
	const std::vector<GLfloat> vertexData
	{
        // 0 - Vertex
		-0.5f, -0.5f, 0.0f, 	// Left vertex position
		1.0f,  0.0f, 0.0f, 	    // color
        // 1 - Vertex
		0.5f, -0.5f, 0.0f,  	// right vertex position
		0.0f,  1.0f, 0.0f,  	// color
        // 2 - Vertex
		-0.5f,  0.5f, 0.0f,  	// Top left vertex position
		0.0f,  0.0f, 1.0f,  	// color
        // 3 - Vertex
		0.5f,  0.5f, 0.0f,  	// Top-right position
		0.0f,  0.0f, 1.0f,  	// color
	};

	// Vertex Arrays Object (VAO) Setup
	glGenVertexArrays(1, &gVertexArrayObject);
	glBindVertexArray(gVertexArrayObject);

	// Vertex Buffer Object (VBO) creation
	glGenBuffers(1, &gVertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER,
				 vertexData.size() * sizeof(GL_FLOAT),
				 vertexData.data(),
				 GL_STATIC_DRAW);
 
    // Index buffer data for a quad
    const std::vector<GLuint> indexBufferData {2,0,1, 3,2,1};
    // Setup the Index Buffer Object
    glGenBuffers(1,&gIndexBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                 gIndexBufferObject);
    // Populate Index Buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indexBufferData.size()*sizeof(GLuint),
                 indexBufferData.data(),
                 GL_STATIC_DRAW);


	glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(GL_FLOAT)*6,
                         (void*)0
    );


    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(GL_FLOAT)*6,
                          (GLvoid*)(sizeof(GL_FLOAT)*3)
            );


	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

/**
* Setup geometry during the vertex specification step
*
* @return void
*/
void VertexSpecification2(){

	const std::vector<GLfloat> vertexDataFloor
	{
        // 0 - Vertex
		-5.0f, -1.0f, -5.0f, 	// Left vertex position
		1.0f,  0.0f, 0.0f, 	    // color
        // 1 - Vertex
		5.0f,  -1.0f, -5.0f,  	// right vertex position
		0.0f,  1.0f, 0.0f,  	// color
        // 2 - Vertex
		-5.0f, -1.0f, 5.0f,  	// Top left vertex position
		0.0f,  1.0f, 0.0f,  	// color
        // 3 - Vertex
		5.0f,  -1.0f, 5.0f,  	// Top-right position
		1.0f,  0.0f, 0.0f,  	// color
	};


	// Vertex Arrays Object (VAO) Setup
	glGenVertexArrays(1, &gVertexArrayObjectFloor);
	glBindVertexArray(gVertexArrayObjectFloor);

	// Vertex Buffer Object (VBO) creation
	glGenBuffers(1, &gVertexBufferObjectFloor);
	glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObjectFloor);
	glBufferData(GL_ARRAY_BUFFER,
				 vertexDataFloor.size() * sizeof(GL_FLOAT),
				 vertexDataFloor.data(),
				 GL_STATIC_DRAW);
 
    // Index buffer data for a quad
    const std::vector<GLuint> indexBufferData {2,0,1, 3,2,1};
    // Setup the Index Buffer Object
    glGenBuffers(1,&gIndexBufferObjectFloor);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                 gIndexBufferObjectFloor);
    // Populate Index Buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indexBufferData.size()*sizeof(GLuint),
                 indexBufferData.data(),
                 GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(GL_FLOAT)*6,
                         (void*)0
    );


    // Color information
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(GL_FLOAT)*6,
                          (GLvoid*)(sizeof(GL_FLOAT)*3)
            );

	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}


/**
* PreDraw
* @return void
*/
void PreDraw(){
	// Disable depth test and face culling.
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // Initialize clear color
    glViewport(0, 0, gScreenWidth, gScreenHeight);
    glClearColor( 0.1f, 4.f, 7.f, 1.f );

    //Clear color buffer and Depth Buffer
  	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // Use shader
	glUseProgram(gGraphicsPipelineShaderProgram);

    // Model transformation by translating object into world space
    glm::mat4 model = glm::translate(glm::mat4(1.0f),glm::vec3(0.0f,0.0f,-5.0f)); 

    // Update model matrix by applying a rotation after translation
    model           = glm::rotate(model,glm::radians(g_uRotate),glm::vec3(0.0f,1.0f,0.0f)); 

    // Projection matrix (in perspective) 
    glm::mat4 perspective = glm::perspective(glm::radians(45.0f),
                                             (float)gScreenWidth/(float)gScreenHeight,
                                             0.1f,
                                             20.0f);
	// end data to GPU
    // Model Matrix
    GLint u_ModelMatrixLocation = glGetUniformLocation(gGraphicsPipelineShaderProgram, "u_ModelMatrix");
    if (u_ModelMatrixLocation >= 0) {
        glUniformMatrix4fv(u_ModelMatrixLocation, 1, GL_FALSE, &model[0][0]);
    } else {
        std::cout << "u_ModelMatrix not found\n";
        exit(EXIT_FAILURE);
    }

    // View Matrix
    GLint u_ViewMatrixLocation = glGetUniformLocation(gGraphicsPipelineShaderProgram, "u_ViewMatrix");
    if (u_ViewMatrixLocation >= 0) {
        glm::mat4 viewMatrix = gCamera.GetViewMatrix();
        glUniformMatrix4fv(u_ViewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
    } else {
        std::cout << "u_ViewMatrix not found\n";
        exit(EXIT_FAILURE);
    }

    // Projection Matrix
    GLint u_ProjectionLocation = glGetUniformLocation(gGraphicsPipelineShaderProgram, "u_Projection");
    if (u_ProjectionLocation >= 0) {
        glUniformMatrix4fv(u_ProjectionLocation, 1, GL_FALSE, &perspective[0][0]);
    } else {
        std::cout << "u_Projection not found\n";
        exit(EXIT_FAILURE);
    }
    // Perform rotation update
    if(g_rotatePositive){
        g_uRotate+=ROTATION_SPEED_UPDATE;
    }else{
        g_uRotate-=ROTATION_SPEED_UPDATE;
    }
}

void PreDrawFloor(){
    // Use shader
	glUseProgram(gGraphicsPipelineShaderProgram);

    // Model transformation by translating object into world space
    glm::mat4 model = glm::translate(glm::mat4(1.0f),glm::vec3(0.0f,0.0f,-5.0f)); 


    // Retrieve location of our Model Matrix
    GLint u_ModelMatrixLocation = glGetUniformLocation( gGraphicsPipelineShaderProgram,"u_ModelMatrix");
    if(u_ModelMatrixLocation >=0){
        glUniformMatrix4fv(u_ModelMatrixLocation,1,GL_FALSE,&model[0][0]);
    }else{
        std::cout << "Could not find u_ModelMatrix, maybe a mispelling?\n";
        exit(EXIT_FAILURE);
    }


    // Update the View Matrix
    GLint u_ViewMatrixLocation = glGetUniformLocation(gGraphicsPipelineShaderProgram,"u_ViewMatrix");
    if(u_ViewMatrixLocation>=0){
        glm::mat4 viewMatrix = gCamera.GetViewMatrix();
        glUniformMatrix4fv(u_ViewMatrixLocation,1,GL_FALSE,&viewMatrix[0][0]);
    }else{
        std::cout << "Could not find u_ModelMatrix, maybe a mispelling?\n";
        exit(EXIT_FAILURE);
    }


    // Projection matrix (in perspective) 
    glm::mat4 perspective = glm::perspective(glm::radians(45.0f),
                                             (float)gScreenWidth/(float)gScreenHeight,
                                             0.1f,
                                             20.0f);

    // Retrieve location of our perspective matrix uniform 
    GLint u_ProjectionLocation= glGetUniformLocation( gGraphicsPipelineShaderProgram,"u_Projection");
    if(u_ProjectionLocation>=0){
        glUniformMatrix4fv(u_ProjectionLocation,1,GL_FALSE,&perspective[0][0]);
    }else{
        std::cout << "Could not find u_Perspective, maybe a mispelling?\n";
        exit(EXIT_FAILURE);
    }

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

void DrawFloor(){
	glBindVertexArray(gVertexArrayObjectFloor);
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObjectFloor);
    glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);
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
    // Two static variables to hold the mouse position
    static int mouseX=gScreenWidth/2;
    static int mouseY=gScreenHeight/2; 

	SDL_Event e;
	while(SDL_PollEvent( &e ) != 0){
		if(e.type == SDL_QUIT){
			std::cout << "Goodbye! (Leaving MainApplicationLoop())" << std::endl;
			gQuit = true;
		}
        if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE){
			std::cout << "ESC: Goodbye! (Leaving MainApplicationLoop())" << std::endl;
            gQuit = true;
        }
        if(e.type==SDL_MOUSEMOTION){
            // Capture the change in the mouse position
            mouseX+=e.motion.xrel * MOUSE_SENSITIVITY;
            mouseY+=e.motion.yrel * MOUSE_SENSITIVITY;
            std::cout << mouseX << "," << mouseY << std::endl;
            gCamera.MouseLook(mouseX,mouseY);
        }
	}

    // Retrieve keyboard state
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_UP]) {
    }
    if (state[SDL_SCANCODE_DOWN]) {
    }
    if (state[SDL_SCANCODE_LEFT]) {
        g_rotatePositive=false;
        std::cout << "g_rotatePositive: " << g_rotatePositive << std::endl;
    }
    if (state[SDL_SCANCODE_RIGHT]) {
        g_rotatePositive=true;
        std::cout << "g_rotatePositive: " << g_rotatePositive << std::endl;
    }

    // Camera
    // Update position of the camera
    if (state[SDL_SCANCODE_W]) {
        gCamera.MoveForward(CAMERA_SPEED);
    }
    if (state[SDL_SCANCODE_S]) {
        gCamera.MoveBackward(CAMERA_SPEED);
    }
    if (state[SDL_SCANCODE_A]) {
        gCamera.MoveLeft(CAMERA_SPEED);
    }
    if (state[SDL_SCANCODE_D]) {
        gCamera.MoveRight(CAMERA_SPEED);
    }
}


/**
* Main Application Loop
*
* @return void
*/
void MainLoop(){

    SDL_WarpMouseInWindow(gGraphicsApplicationWindow,gScreenWidth/2,gScreenHeight/2);
    SDL_SetRelativeMouseMode(SDL_TRUE);

	while(!gQuit){
		Input();
		PreDraw();
		Draw();

		PreDrawFloor();
		DrawFloor();

		SDL_GL_SwapWindow(gGraphicsApplicationWindow);
	}
}



/**
* Destroy any global objects
*
* @return void
*/
void CleanUp(){
	//Destroy SDL2 Window
	SDL_DestroyWindow(gGraphicsApplicationWindow );
	gGraphicsApplicationWindow = nullptr;

    // Delete OpenGL Objects
    glDeleteBuffers(1, &gVertexBufferObject);
    glDeleteVertexArrays(1, &gVertexArrayObject);

	// Delete Graphics pipeline
    glDeleteProgram(gGraphicsPipelineShaderProgram);

	//Quit SDL subsystems
	SDL_Quit();
}


/**
* The entry point.
*
* @return program status
*/
int main( int argc, char* args[] ){
    std::cout << "Use wasd keys to move mouse to rotate\n";
    std::cout << "Press ESC to quit\n";

	InitializeProgram();
	
	VertexSpecification();
	VertexSpecification2();
	
	CreateGraphicsPipeline();
	
	MainLoop();	

	CleanUp();

	return 0;
}
