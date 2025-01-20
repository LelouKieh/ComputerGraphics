#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <vector>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 

class Object {
public:
    Object(const std::string& filepath);
    ~Object();

    void Initialize();
    void PreDraw();
    void Draw();

    std::vector<float> GetVertices() const;
    std::vector<float> GetNormals() const;
    std::vector<float> GetTexCoords() const;

private:
    void ComputeNormals();
    void LoadOBJ(const std::string& filepath);

    // Data
    std::vector<float> mVertices;
    std::vector<float> mNormals;
    std::vector<float> mTexCoords;
    std::vector<unsigned int> mIndices; 

    // OpenGL objects
    GLuint mVAO;
    GLuint mVBO[3];
    GLuint mEBO;
    GLuint mShaderID;
};

#endif