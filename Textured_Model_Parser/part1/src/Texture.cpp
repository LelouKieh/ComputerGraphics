#if defined(LINUX) || defined(MINGW)
    #include <SDL2/SDL.h>
#else // This works for Mac
    #include <SDL.h>
#endif


#include "Texture.hpp"

#include <stdio.h>
#include <string.h>

#include <fstream>
#include <iostream>
#include <glad/glad.h>
#include <memory>

// Default Constructor
Texture::Texture(){

}


// Default Destructor
Texture::~Texture(){
	// Delete our texture from the GPU
	glDeleteTextures(1,&m_textureID);

    // Delete our image
    if(m_image != nullptr){
        delete m_image;
    }

}

void Texture::LoadTexture(const std::string filepath){
	// Set member variable
    m_filepath = filepath;
    // Load our actual image data
    m_image = new Image(filepath);
    m_image->LoadPPM(true);
	std::cout << "Loading texture: " << filepath << std::endl;

    glEnable(GL_TEXTURE_2D); 
		// Generate a buffer for our texture
    glGenTextures(1,&m_textureID);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
	 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
	 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
		// Wrap mode describes what to do if we go outside the boundaries of texture.
  	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
		glTexImage2D(GL_TEXTURE_2D,
							0 ,
	  					GL_RGB,
                        m_image->GetWidth(),
                        m_image->GetHeight(),
		  				0,
			  			GL_RGB,
				  		GL_UNSIGNED_BYTE,
					  	m_image->GetPixelDataPtr());
    // Generate a mipmap
    glGenerateMipmap(GL_TEXTURE_2D);                          
		glBindTexture(GL_TEXTURE_2D, 0);
}


void Texture::Bind(unsigned int slot) const{
  glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0+slot);
	glBindTexture(GL_TEXTURE_2D, m_textureID);
}

void Texture::Unbind(){
	glBindTexture(GL_TEXTURE_2D, 0);
}


