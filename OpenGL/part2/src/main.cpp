#if defined(LINUX) || defined(MINGW)
    #include <SDL2/SDL.h>
#else // This works for Mac
    #include <SDL.h>
#endif

// Third Party Libraries
#include <glad/glad.h>

// C++ Standard Template Library (STL)
#include <iostream>
#include <vector>


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
// Vertex Buffer Object (VBO)
GLuint 	gVertexBufferObject					= 0;
// Element Buffer Object (EBO)
GLuint gElementBufferObject                 = 0;

// Number of indices
int gNumberOfIndicesToDraw = 3; // draw a triangle

// Shaders

// Vertex Shader
const std::string gVertexShaderSource =
	"#version 410 core\n"
	"in vec4 position;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = vec4(position.x, position.y, position.z, position.w);\n"
	"}\n";

// Fragment Shader
const std::string gFragmentShaderSource =
	"#version 410 core\n"
	"out vec4 color;\n"
	"void main()\n"
	"{\n"
	"	color = vec4(1.0f, 0.5f, 0.0f, 1.0f);\n"
	"}\n";


/**
* CompileShader will compile any valid vertex, fragment, geometry, tesselation, or compute shader.
*
* @param type determine which shader to compile.
* @param source : The shader source code.
* @return id of the shaderObject
*/
GLuint CompileShader(GLuint type, const std::string& source){
	// Compile our shaders
	GLuint shaderObject;

	// Based on the type passed in, create a shader object type.
	if(type == GL_VERTEX_SHADER){
		shaderObject = glCreateShader(GL_VERTEX_SHADER);
	}else if(type == GL_FRAGMENT_SHADER){
		shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	}

	const char* src = source.c_str();
	// The source of our shader
	glShaderSource(shaderObject, 1, &src, nullptr);
	// Now compile shader
	glCompileShader(shaderObject);

	// Retrieve the result of our compilation
	int result;
	// to retrieve the compilation status
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
	// Delete the individual shaders
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
	gGraphicsPipelineShaderProgram = CreateShaderProgram(gVertexShaderSource,gFragmentShaderSource);
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
	const std::vector<GLfloat> vertexPositions
	{
		-0.8f, -0.8f, 0.0f, 	// Left vertex position
		0.8f, -0.8f, 0.0f,  	// right vertex position
		0.8f,  0.8f, 0.0f,  	// Top right vertex position
		-0.8f,  0.8f, 0.0f      // Top left vertex position
	};

	// Vertex Arrays Object (VAO) Setup
	glGenVertexArrays(1, &gVertexArrayObject);
	glBindVertexArray(gVertexArrayObject);

	// Vertex Buffer Object (VBO) creation
	glGenBuffers(1, &gVertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER,
				 vertexPositions.size() * sizeof(GL_FLOAT),
				 vertexPositions.data(),
				 GL_STATIC_DRAW);


    // Setup indices
    const std::vector<GLuint> indexBufferData { 
		0, 1, 2, // 1st triangle
		2, 3, 0  // 2nd triangle
		};
	glGenBuffers(1, &gElementBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gElementBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,indexBufferData.size() * sizeof(GLuint),indexBufferData.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          0,
                          (void*)0
    );

	// Unbind
	glBindVertexArray(0);
	// Disable attributes
	glDisableVertexAttribArray(0);
}


/**
* PreDraw
* 
* @return void
*/
void PreDraw(){
	// Disable depth test and face culling.
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // Initialize clear color
    glViewport(0, 0, gScreenWidth, gScreenHeight);
    glClearColor( 1.f, 1.f, 0.f, 1.f );

    //Clear color buffer and Depth Buffer
  	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // Use shader
	glUseProgram(gGraphicsPipelineShaderProgram);
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
	glDrawElements(GL_TRIANGLES, gNumberOfIndicesToDraw, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);

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
* Function called in the main application loop to handle user input
*
* @return void
*/
void Input(){
	SDL_Event e;
	//Handle events on queue
	while(SDL_PollEvent( &e ) != 0){
		if(e.type == SDL_QUIT){
			std::cout << "Goodbye! (Leaving MainApplicationLoop())" << std::endl;
			gQuit = true;
		} else if (e.type == SDL_KEYDOWN) {
			switch(e.key.keysym.sym){
                case SDLK_LEFT:
                    gNumberOfIndicesToDraw = 3; // Draw triangle
                    break;
                case SDLK_RIGHT:
                    gNumberOfIndicesToDraw = 6; // Draw square
                    break;
            }
		}
	}
}


/**
* Main Application Loop
*
* @return void
*/
void MainLoop(){

	while(!gQuit){
		// Handle Input
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
    glDeleteBuffers(1, &gVertexBufferObject);
	glDeleteBuffers(1, &gElementBufferObject);
    glDeleteVertexArrays(1, &gVertexArrayObject);

	// Delete Graphics pipeline
    glDeleteProgram(gGraphicsPipelineShaderProgram);

	//Quit SDL subsystems
	SDL_Quit();
}


/**
* The main entry point.
*
* @return program status
*/
int main( int argc, char* args[] ){

	// Setup the graphics program
	InitializeProgram();
	
	// Setup geometry
	VertexSpecification();
	
	// Create graphics pipeline
	CreateGraphicsPipeline();
	
	// Call the main application loop
	MainLoop();	

	// Call the cleanup function
	CleanUp();

	return 0;
}
