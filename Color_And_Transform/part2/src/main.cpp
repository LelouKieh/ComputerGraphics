
/* Compilation on Linux: 
 g++ -std=c++17 ./src/*.cpp -o prog -I ./include/ -I./../common/thirdparty/ -lSDL2 -ldl
*/

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

// Screen Dimensions
int gScreenWidth 						= 640;
int gScreenHeight 						= 480;
SDL_Window* gGraphicsApplicationWindow 	= nullptr;
SDL_GLContext gOpenGLContext			= nullptr;

// Main loop flag
bool gQuit = false;

// shader
GLuint gGraphicsPipelineShaderProgram	= 0;
GLuint gGraphicsPipelineShaderProgramDebug	= 0;

// OpenGL Objects
// Vertex Array Object (VAO)
GLuint gVertexArrayObject					= 0;
// Vertex Buffer Object (VBO)
GLuint 	gVertexBufferObject					= 0;
// Index Buffer Object (IBO)
GLuint 	gIndexBufferObject                  = 0;


// A second object for drawing a normal
GLuint gVertexArrayObjectForNormal					= 0;
// Vertex Buffer Object (VBO)
GLuint 	gVertexBufferObjectForNormal					= 0;

float g_uOffset=0.0f;


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
	// Compile shaders
	GLuint shaderObject;

	// create a shader object
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
* Creates a graphics program object with a Vertex Shader and a Fragment Shader
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

    // Link two shader programs together
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

    std::string vertexDebugShaderSource      = LoadShaderAsString("./shaders/vert.glsl");
    std::string fragmentDebugShaderSource    = LoadShaderAsString("./shaders/debug_frag.glsl");

	gGraphicsPipelineShaderProgramDebug = CreateShaderProgram(vertexDebugShaderSource,fragmentDebugShaderSource);
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


// helper function for creating edges from two vertices
glm::vec3 CreateEdgeFromTwoVertices(float x1, float y1, float z1,
									float x2, float y2, float z2){
	return glm::vec3(x2-x1,y2-y1,z2-z1);	
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
				-0.5f, -0.5f, -3.5f, 	// Left vertex position
        // 1 - Vertex
				0.5f, -0.5f, -1.5f,  	// right vertex position
        // 2 - Vertex
				-0.0f,  0.5f, -2.5f,  	// Top left vertex position
	};
	/*
	// Another test case
	const std::vector<GLfloat> vertexData
	{
        // 0 - Vertex
				-0.5f, -0.4f, -1.0f, 	// Left vertex position
        // 1 - Vertex
				0.5f, -0.4f, -1.0f,  	// right vertex position
        // 2 - Vertex
				-0.0f,  -0.4f, -9.0f,  	// Top left vertex position
	};
	*/

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
    const std::vector<GLuint> indexBufferData {0,1,2};
    // Setup the Index Buffer Object (IBO i.e. EBO)
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
                          sizeof(GL_FLOAT)*3,
                         (void*)0
    );
	glBindVertexArray(0);
	glDisableVertexAttribArray(0);



	// Container for holding the surface normal
	std::vector<GLfloat> surfaceNormalData;

	// Vertices from vertexData
    glm::vec3 v0(vertexData[0], vertexData[1], vertexData[2]);
    glm::vec3 v1(vertexData[3], vertexData[4], vertexData[5]);
    glm::vec3 v2(vertexData[6], vertexData[7], vertexData[8]);

	// Create edges e1 and e2
    glm::vec3 e1 = CreateEdgeFromTwoVertices(v0.x, v0.y, v0.z, v1.x, v1.y, v1.z);
    glm::vec3 e2 = CreateEdgeFromTwoVertices(v0.x, v0.y, v0.z, v2.x, v2.y, v2.z);
	
	// Take cross product to get perpendicular edge 
	glm::vec3 normal = glm::normalize(glm::cross(e1, e2));

	// compute the 'midpoint' or 'centroid' so the normal shows up towards the center of triangle.
	glm::vec3 midpoint = (v0 + v1 + v2) / 3.0f; 

	// Populate the surface normal data
  	surfaceNormalData.push_back(midpoint.x);
  	surfaceNormalData.push_back(midpoint.y);
  	surfaceNormalData.push_back(midpoint.z);
 	surfaceNormalData.push_back(midpoint.x+normal.x);
 	surfaceNormalData.push_back(midpoint.y+normal.y);
  	surfaceNormalData.push_back(midpoint.z+normal.z);
	

	glGenVertexArrays(1, &gVertexArrayObjectForNormal);
	glBindVertexArray(gVertexArrayObjectForNormal);

	// Vertex Buffer Object (VBO) creation
	glGenBuffers(1, &gVertexBufferObjectForNormal);
	glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObjectForNormal);
	glBufferData(GL_ARRAY_BUFFER,
				 surfaceNormalData.size() * sizeof(GL_FLOAT),
				 surfaceNormalData.data(),
				 GL_STATIC_DRAW);


	glEnableVertexAttribArray(0);
    	glVertexAttribPointer(0,
                    3,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(GL_FLOAT)*3,
                         (void*)0
    );

	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
}

/**
* PreDraw
* @return void
*/
void PreDraw(){
	// Disable depth test and face culling.
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // Initialize clear color
    glViewport(0, 0, gScreenWidth, gScreenHeight);
    glClearColor( 0.5f, 9.f, 1.f, 1.f );

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
    // Use shader
		glUseProgram(gGraphicsPipelineShaderProgram);
    // Projection matrix (in perspective) 
    glm::mat4 perspective = glm::perspective(glm::radians(45.0f),
                                             (float)gScreenWidth/(float)gScreenHeight,
                                             0.1f,
                                             10.0f);

    // Retrieve location of perspective matrix uniform 
    GLint u_ProjectionLocation= glGetUniformLocation( gGraphicsPipelineShaderProgram,"u_Projection");
    if(u_ProjectionLocation>=0){
        glUniformMatrix4fv(u_ProjectionLocation,1,GL_FALSE,&perspective[0][0]);
    }else{
        std::cout << "Could not find u_Projection, maybe a mispelling?\n";
        exit(EXIT_FAILURE);
    }

    // Enable attributes
		glBindVertexArray(gVertexArrayObject);

    //Render data
    glDrawElements(GL_TRIANGLES,
                    3,
                    GL_UNSIGNED_INT,
                    0);



    // Draw a line for normal
		glUseProgram(gGraphicsPipelineShaderProgramDebug);
    u_ProjectionLocation= glGetUniformLocation( gGraphicsPipelineShaderProgramDebug,"u_Projection");
    if(u_ProjectionLocation>=0){
        glUniformMatrix4fv(u_ProjectionLocation,1,GL_FALSE,&perspective[0][0]);
    }else{
        std::cout << "Could not find u_Projection, maybe a mispelling?\n";
        exit(EXIT_FAILURE);
    }
	glBindVertexArray(gVertexArrayObjectForNormal);
	glDrawArrays(GL_LINES,0,2);

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
	while(SDL_PollEvent( &e ) != 0){
		if(e.type == SDL_QUIT){
			std::cout << "Goodbye! (Leaving MainApplicationLoop())" << std::endl;
			gQuit = true;
		}
	}

    // Retrieve keyboard state
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_UP]) {
        g_uOffset+=0.01f;
        std::cout << "g_uOffset: " << g_uOffset << std::endl;
    }
    if (state[SDL_SCANCODE_DOWN]) {
        g_uOffset-=0.01f;
        std::cout << "g_uOffset: " << g_uOffset << std::endl;
    }

}


/**
* Main Application Loop
*
* @return void
*/
void MainLoop(){

	while(!gQuit){
		Input();
		PreDraw();
		Draw();
		SDL_GL_SwapWindow(gGraphicsApplicationWindow);
	}
}



/**
* destroy any global objects.
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

	InitializeProgram();

	VertexSpecification();
	
	CreateGraphicsPipeline();

	MainLoop();	

	CleanUp();

	return 0;
}
