#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#if defined(LINUX) || defined(MINGW)
    #include <SDL2/SDL.h>
#else
    #include <SDL.h>
#endif

#include <string>
#include "Camera.hpp"
#include "Object.hpp"
#include "Texture.hpp"
#include "Image.hpp"
#include "Light.hpp"


struct Global{
		// Screen Dimensions
		int gScreenWidth 						= 640;
		int gScreenHeight 						= 480;
		SDL_Window* gGraphicsApplicationWindow 	= nullptr;
		SDL_GLContext gOpenGLContext			= nullptr;

		// shader
		GLuint gGraphicsPipelineShaderProgram	= 0;

		// Main loop flag
		bool gQuit = false;
		
		bool gWireframeMode = false;

		// Camera
		Camera gCamera;

		// Texture
		Texture gTexture;
		Texture gNormalMap;

		// Draw wireframe mode
		GLenum gPolygonMode = GL_FILL;

		// Object to render
		Object* gObject = nullptr;
		
		// OBJ file path
		std::string objFilePath;

		Light gLight;
		
		float g_uOffset=-2.0f;
		float g_uRotate=0.0f;
};

extern Global g;

#endif
