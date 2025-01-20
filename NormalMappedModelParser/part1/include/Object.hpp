#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <iostream>
#include <vector>
#include <string>
#include <glad/glad.h>
#include "Texture.hpp"
#include <glm/glm.hpp>

class Object {
private:
    std::string mFilepath;
    std::string mDirectory;
    std::vector<glm::vec3> mVertices;
    std::vector<glm::vec2> mTexCoords;
    std::vector<glm::vec3> mNormals;

    // Indices
    std::vector<unsigned int> mIndices;
    glm::vec3 mPosition;

    // OpenGL buffers and objects
    GLuint mVAO = 0;
    GLuint mVBO_Vertices = 0;
    GLuint mVBO_TexCoords = 0;
    GLuint mVBO_Normals = 0;
    GLuint mEBO = 0; // Element Buffer Object (for indices)
    GLuint mVBO_Tangents = 0;
    GLuint mVBO_Bitangents = 0;

    // Texture
    Texture mTexture;
    Texture mNormalMapTexture;
    std::string mTextureFilepath;
    std::vector<glm::vec3> mTangents;
    std::vector<glm::vec3> mBitangents;

    // Parse functions
    void parseOBJ(const std::string& filepath);
    void parseMTL(const std::string& filepath);

public:
    Object(const std::string& filepath);
    ~Object();
    void Initialize();
    void PreDraw();
    void Draw();
    void ComputeTangentSpace();
};

#endif
