#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#if defined(LINUX) || defined(MINGW)
    #include <SDL2/SDL.h>
#else // This works for Mac
    #include <SDL.h>
#endif

#include "STL.hpp"
#include <string>
#include "Camera.hpp"
#include "Light.hpp"
#include "Object.hpp"

// Forward Declaration
struct STLFile;

struct Global{
		// Screen Dimensions
		int gScreenWidth 						= 640;
		int gScreenHeight 						= 480;
		SDL_Window* gGraphicsApplicationWindow 	= nullptr;
		SDL_GLContext gOpenGLContext			= nullptr;

		// Main loop flag
		bool gQuit = false;

		// Camera
		Camera gCamera;

		// Draw wireframe mode
		GLenum gPolygonMode = GL_FILL;

		// Light object
		Light gLight;

		// Object to render
		Object* gObject = nullptr; // Replace STLFile with Object
		
		// OBJ file path
		std::string objFilePath;
		
		// 3D object -- a bunny for the purpose of this demo
		STLFile* gBunny;
};

extern Global g;

#endif
