#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include "Image.hpp"

#include <glad/glad.h>
#include <string>

class Texture{
public:
    // Constructor
    Texture();
    // Destructor
    ~Texture();
	// Loads and sets up an actual texture
    void LoadTexture(const std::string filepath);
    void Bind(unsigned int slot=0) const;
    void Unbind();
private:
    // Store a unique ID for the texture
    GLuint m_textureID;
	// Filepath to the image loaded
    std::string m_filepath;
    // Store image data inside our texture class.
    Image* m_image;
};



#endif