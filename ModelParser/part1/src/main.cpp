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
GLuint gVertexArrayObjectFloor= 0;
// Vertex Buffer Object (VBO)
GLuint  gVertexBufferObjectFloor            = 0;

// Camera
Camera gCamera;

// Draw wireframe mode
// Set default to filled mode
GLenum gPolygonMode = GL_FILL;

// Floor resolution
size_t gFloorResolution = 10;
size_t gFloorTriangles  = 0;



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
	// Now compile shader
	glCompileShader(shaderObject);

	// Retrieve the result of compilation
	int result;
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
	// Use OpenGL 4.1 core or greater
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
	// We want to request a double buffer for smooth updating.
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	// Create an application window using OpenGL that supports SDL
	gGraphicsApplicationWindow = SDL_CreateWindow( "Tesselation",
													SDL_WINDOWPOS_UNDEFINED,
													SDL_WINDOWPOS_UNDEFINED,
													gScreenWidth,
													gScreenHeight,
													SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );

	// Check if Window created.
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


struct Vertex{
    float x,y,z;    // position
	float r,g,b; 	// color
	float nx,ny,nz; // normals
};  

struct Triangle{
    Vertex vertices[3]; // 3 vertices per triangle
};



// Return a value that is a mapping between the current range and a new range.
float map_linear(float x, float in_min, float in_max, float out_min, float out_max){
    return (x-in_min) * (out_max - out_min) / (in_max - in_min) + out_min;;
}

// Pass in an unsigned integer representing the number of rows and columns in the plane
std::vector<Triangle> generatePlane(size_t resolution=0){
    // Store the resulting plane
    std::vector<Triangle> result;

    // If resolution is 0, return empty vector
    if (resolution == 0)
        return result;

    // Generate the vertices
    std::vector<std::vector<Vertex>> vertices(resolution + 1, std::vector<Vertex>(resolution + 1));

    // For i from 0 to resolution
    for (size_t i = 0; i <= resolution; ++i) {
        for (size_t j = 0; j <= resolution; ++j) {
            float x = map_linear(i, 0, resolution, -1.0f, 1.0f);
            float z = map_linear(j, 0, resolution, -1.0f, 1.0f);
            float y = -0.5f;

            Vertex v;
            v.x = x;
            v.y = y;
            v.z = z;
            // Set color to dark green
            v.r = 0.0f;
            v.g = 0.5f;
            v.b = 0.0f;

            vertices[i][j] = v;
        }
    }

    // generate triangles
    for (size_t i = 0; i < resolution; ++i) {
        for (size_t j = 0; j < resolution; ++j) {
            // First triangle
            Triangle tri1;
            tri1.vertices[0] = vertices[i][j];
            tri1.vertices[1] = vertices[i + 1][j];
            tri1.vertices[2] = vertices[i + 1][j + 1];

            // Second triangle
            Triangle tri2;
            tri2.vertices[0] = vertices[i][j];
            tri2.vertices[1] = vertices[i + 1][j + 1];
            tri2.vertices[2] = vertices[i][j + 1];

            result.push_back(tri1);
            result.push_back(tri2);
        }
    }

    return result;
}




// Regenerate the flat plane
void GeneratePlaneBufferData(){
    // Generate a plane with the resolution 
    std::vector<Triangle> mesh = generatePlane(gFloorResolution); 

	std::vector<GLfloat> vertexDataFloor;
    // For each triangle
    for (const Triangle& tri : mesh) {
        // For each vertex in triangle
        for (int i = 0; i < 3; ++i) {
            const Vertex& v = tri.vertices[i];
            // position x, y, z
            vertexDataFloor.push_back(v.x);
            vertexDataFloor.push_back(v.y);
            vertexDataFloor.push_back(v.z);
            // color r, g, b
            vertexDataFloor.push_back(v.r);
            vertexDataFloor.push_back(v.g);
            vertexDataFloor.push_back(v.b);
            // normal nx, ny, nz
            vertexDataFloor.push_back(v.nx);
            vertexDataFloor.push_back(v.ny);
            vertexDataFloor.push_back(v.nz);
        }
    }

    // Store size in a global so you can later determine how many vertices to draw in glDrawArrays
    gFloorTriangles = vertexDataFloor.size() / 9; // Number of vertices


		glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObjectFloor);
		glBufferData(GL_ARRAY_BUFFER,
							 vertexDataFloor.size() * sizeof(GL_FLOAT),
							 vertexDataFloor.data(),
							 GL_STATIC_DRAW);
}

/**
* Setup geometry during the vertex specification step
*
* @return void
*/
void VertexSpecification(){

	// Vertex Arrays Object (VAO) Setup
	glGenVertexArrays(1, &gVertexArrayObjectFloor);
	glBindVertexArray(gVertexArrayObjectFloor);
	// Vertex Buffer Object (VBO) creation
	glGenBuffers(1, &gVertexBufferObjectFloor);

    // Generate our data for the buffer
    GeneratePlaneBufferData();
 
	glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,sizeof(GL_FLOAT)*9,(void*)0);
    // Color information (r,g,b)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,sizeof(GL_FLOAT)*9,(GLvoid*)(sizeof(GL_FLOAT)*3));
    // Normal information (nx,ny,nz)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,sizeof(GL_FLOAT)*9, (GLvoid*)(sizeof(GL_FLOAT)*6));

	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}


/**
* PreDraw
* @return void
*/
void PreDraw(){
    // In PreDraw()
    GLuint objectColorLoc = glGetUniformLocation(gGraphicsPipelineShaderProgram, "objectColor");
    // Set the object color (red)
    glUniform3f(objectColorLoc, 1.0f, 0.0f, 0.0f); // RGB values for red
	// Disable depth test and face culling.
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // Set the polygon fill mode
    glPolygonMode(GL_FRONT_AND_BACK,gPolygonMode);

    // Initialize clear color
    glViewport(0, 0, gScreenWidth, gScreenHeight);
    glClearColor( 0.1f, 4.f, 7.f, 1.f );

    //Clear color buffer and Depth Buffer
  	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // Use shader
	glUseProgram(gGraphicsPipelineShaderProgram);

    // Model transformation by translating object into world space
    glm::mat4 model = glm::translate(glm::mat4(1.0f),glm::vec3(0.0f,0.0f,0.0f)); 


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

    // Retrieve location of perspective matrix uniform 
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
	glBindVertexArray(gVertexArrayObjectFloor);

    //Render data
    glDrawArrays(GL_TRIANGLES,0,gFloorTriangles);

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
            mouseX+=e.motion.xrel;
            mouseY+=e.motion.yrel;
            gCamera.MouseLook(mouseX,mouseY);
        }
	}

    // Retrieve keyboard state
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_UP]) {
        SDL_Delay(250);
        gFloorResolution+=1;
        std::cout << "Resolution:" << gFloorResolution << std::endl;
        GeneratePlaneBufferData();
    }
    if (state[SDL_SCANCODE_DOWN]) {
        SDL_Delay(250); 
        gFloorResolution-=1;
        if(gFloorResolution<=1){
            gFloorResolution=1;
        }
        std::cout << "Resolution:" << gFloorResolution << std::endl;
        GeneratePlaneBufferData();
    }

    // Camera
    // Update our position of the camera
    if (state[SDL_SCANCODE_W]) {
        gCamera.MoveForward(0.002f);
    }
    if (state[SDL_SCANCODE_S]) {
        gCamera.MoveBackward(0.002f);
    }
    if (state[SDL_SCANCODE_A]) {
    }
    if (state[SDL_SCANCODE_D]) {
    }

    if (state[SDL_SCANCODE_TAB]) {
        SDL_Delay(250);
        if(gPolygonMode== GL_FILL){
            gPolygonMode = GL_LINE;
        }else{
            gPolygonMode = GL_FILL;
        }
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
		SDL_GL_SwapWindow(gGraphicsApplicationWindow);
	}
}



/**
* Destroy any global objects.
*
* @return void
*/
void CleanUp(){
	//Destroy SDL2 Window
	SDL_DestroyWindow(gGraphicsApplicationWindow );
	gGraphicsApplicationWindow = nullptr;

    // Delete OpenGL Objects
    glDeleteBuffers(1, &gVertexBufferObjectFloor);
    glDeleteVertexArrays(1, &gVertexArrayObjectFloor);

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
    std::cout << "Use w and s keys to move forward and back\n";
    std::cout << "Use up and down to change tessellation\n";
    std::cout << "Use 1 to toggle wireframe\n";
    std::cout << "Press ESC to quit\n";

	InitializeProgram();
	
	VertexSpecification();
	
	CreateGraphicsPipeline();
	
	MainLoop();	

	CleanUp();

	return 0;
}
