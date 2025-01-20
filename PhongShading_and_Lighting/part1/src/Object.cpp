#include "Object.hpp"
#include "util.hpp"
#include "globals.hpp"

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <functional>
#include <string>

// Define the Vertex struct
struct Vertex {
    glm::vec3 position;
    glm::vec2 texCoord;
    glm::vec3 normal;

    bool operator==(const Vertex& other) const {
        return position == other.position &&
               texCoord == other.texCoord &&
               normal == other.normal;
    }
};

// Provide a hash function for Vertex to be used in unordered_map
namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(const Vertex& vertex) const {
            size_t h1 = hash<float>{}(vertex.position.x) ^ (hash<float>{}(vertex.position.y) << 1) ^ (hash<float>{}(vertex.position.z) << 2);
            size_t h2 = hash<float>{}(vertex.texCoord.x) ^ (hash<float>{}(vertex.texCoord.y) << 1);
            size_t h3 = hash<float>{}(vertex.normal.x) ^ (hash<float>{}(vertex.normal.y) << 1) ^ (hash<float>{}(vertex.normal.z) << 2);
            return h1 ^ h2 ^ h3;
        }
    };
}

Object::Object(const std::string& filepath) {
    LoadOBJ(filepath);
}

Object::~Object() {
    // Delete OpenGL objects
    glDeleteBuffers(3, mVBO);
    glDeleteVertexArrays(1, &mVAO);
    glDeleteProgram(mShaderID);
}

void Object::ComputeNormals() {
    // Initialize normals
    mNormals.resize(mVertices.size(), 0.0f);

    // Iterate over each face (triangle)
    for (size_t i = 0; i < mIndices.size(); i += 3) {
        unsigned int index0 = mIndices[i];
        unsigned int index1 = mIndices[i + 1];
        unsigned int index2 = mIndices[i + 2];

        // Get the vertex positions
        glm::vec3 v0(mVertices[3 * index0], mVertices[3 * index0 + 1], mVertices[3 * index0 + 2]);
        glm::vec3 v1(mVertices[3 * index1], mVertices[3 * index1 + 1], mVertices[3 * index1 + 2]);
        glm::vec3 v2(mVertices[3 * index2], mVertices[3 * index2 + 1], mVertices[3 * index2 + 2]);

        // Compute face normal
        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

        // Add the face normal to each vertex normal (for smooth shading)
        for (int j = 0; j < 3; ++j) {
            mNormals[3 * index0 + j] += faceNormal[j];
            mNormals[3 * index1 + j] += faceNormal[j];
            mNormals[3 * index2 + j] += faceNormal[j];
        }
    }

    // Normalize the normals
    for (size_t i = 0; i < mNormals.size(); i += 3) {
        glm::vec3 normal(mNormals[i], mNormals[i + 1], mNormals[i + 2]);
        normal = glm::normalize(normal);
        mNormals[i] = normal.x;
        mNormals[i + 1] = normal.y;
        mNormals[i + 2] = normal.z;
    }
}

void Object::LoadOBJ(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open OBJ file: " << filepath << std::endl;
        exit(EXIT_FAILURE);
    }

    // Temporary storage
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_texCoords;
    std::vector<glm::vec3> temp_normals;

    std::vector<Vertex> uniqueVertices;
    std::vector<unsigned int> indices;

    std::unordered_map<Vertex, unsigned int> vertexToIndex;

    std::string line;
    while (getline(file, line)) {
        std::istringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "v") {
            // Vertex position
            glm::vec3 vertex;
            ss >> vertex.x >> vertex.y >> vertex.z;
            temp_vertices.push_back(vertex);
        } else if (prefix == "vt") {
            // Texture coordinate
            glm::vec2 texCoord;
            ss >> texCoord.x >> texCoord.y;
            temp_texCoords.push_back(texCoord);
        } else if (prefix == "vn") {
            // Normal vector
            glm::vec3 normal;
            ss >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        } else if (prefix == "f") {
            // Face
            std::string vertexData;
            std::vector<std::string> faceVertices;
            while (ss >> vertexData) {
                faceVertices.push_back(vertexData);
            }

            if (faceVertices.size() != 3) {
                std::cerr << "Error: Only triangular faces are supported.\n";
                exit(EXIT_FAILURE);
            }

            for (const auto& v : faceVertices) {
                unsigned int vertexIndex = 0, texCoordIndex = 0, normalIndex = 0;

                if (v.find("//") != std::string::npos) {
                    // Format: v//vn (no texture coordinate)
                    sscanf(v.c_str(), "%u//%u", &vertexIndex, &normalIndex);
                    texCoordIndex = 0; // Indicate missing texture coordinate
                } else if (v.find('/') != std::string::npos) {
                    // Format: v/vt/vn or v/vt
                    int matches = sscanf(v.c_str(), "%u/%u/%u", &vertexIndex, &texCoordIndex, &normalIndex);
                    if (matches == 2) {
                        normalIndex = 0; // Indicate missing normal
                    }
                } else {
                    // Format: v (only vertex indices)
                    sscanf(v.c_str(), "%u", &vertexIndex);
                    texCoordIndex = 0;
                    normalIndex = 0;
                }

                // Adjust indices (OBJ indices start at 1)
                vertexIndex = vertexIndex - 1;
                if (texCoordIndex > 0) texCoordIndex = texCoordIndex - 1;
                if (normalIndex > 0) normalIndex = normalIndex - 1;

                // Retrieve vertex attributes
                glm::vec3 position = temp_vertices[vertexIndex];
                glm::vec2 texCoord = (texCoordIndex < temp_texCoords.size()) ? temp_texCoords[texCoordIndex] : glm::vec2(0.0f);
                glm::vec3 normal = (normalIndex < temp_normals.size()) ? temp_normals[normalIndex] : glm::vec3(0.0f);

                // Create a vertex
                Vertex vertex = { position, texCoord, normal };

                // Check if vertex is already in the uniqueVertices list
                if (vertexToIndex.count(vertex) == 0) {
                    uniqueVertices.push_back(vertex);
                    vertexToIndex[vertex] = static_cast<unsigned int>(uniqueVertices.size() - 1);
                }

                indices.push_back(vertexToIndex[vertex]);
            }
        }
    }
    file.close();

    // Separate the unique vertex data into separate arrays
    mVertices.clear();
    mNormals.clear();
    mTexCoords.clear();
    for (const auto& vertex : uniqueVertices) {
        mVertices.push_back(vertex.position.x);
        mVertices.push_back(vertex.position.y);
        mVertices.push_back(vertex.position.z);

        mTexCoords.push_back(vertex.texCoord.x);
        mTexCoords.push_back(vertex.texCoord.y);

        mNormals.push_back(vertex.normal.x);
        mNormals.push_back(vertex.normal.y);
        mNormals.push_back(vertex.normal.z);
    }

    mIndices = indices;

    // If normals are missing, compute them
    if (temp_normals.empty()) {
        ComputeNormals();
    }
    std::cout << mVertices.size() << " vertices\n";
    std::cout << mNormals.size() << " normals\n";
    std::cout << mTexCoords.size() << " texture coordinates\n";
}

void Object::Initialize() {
    std::string vertexShaderSource = LoadShaderAsString("./shaders/vert.glsl");
    std::string fragmentShaderSource = LoadShaderAsString("./shaders/frag.glsl");

    mShaderID = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);

    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);

    // Positions
    glGenBuffers(1, &mVBO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(float), mVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    // Normals
    glGenBuffers(1, &mVBO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO[1]);
    glBufferData(GL_ARRAY_BUFFER, mNormals.size() * sizeof(float), mNormals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

    // Reuse Normals as Colors
    glGenBuffers(1, &mVBO[2]);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO[2]);
    glBufferData(GL_ARRAY_BUFFER, mNormals.size() * sizeof(float), mNormals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(2);

    // Element Buffer Object (EBO)
    glGenBuffers(1, &mEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * sizeof(unsigned int), mIndices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}

void Object::PreDraw() {
    // Use our shader
	glUseProgram(mShaderID);

    // Model transformation by translating our object into world space
    glm::mat4 model = glm::translate(glm::mat4(1.0f),glm::vec3(0.0f,0.0f,0.0f)); 
    static float rot=0.0f;
    rot += 0.1f; // Uncomment to add a rotation
    model = glm::rotate(model,glm::radians(rot),glm::vec3(0.0f,1.0f,0.0f)); 

    // Retrieve our location of our Model Matrix
    GLint u_ModelMatrixLocation = glGetUniformLocation( mShaderID,"u_ModelMatrix");
    if(u_ModelMatrixLocation >=0){
        glUniformMatrix4fv(u_ModelMatrixLocation,1,GL_FALSE,&model[0][0]);
    }else{
        std::cout << "Could not find u_ModelMatrix, maybe a mispelling?\n";
        exit(EXIT_FAILURE);
    }

    // Update the View Matrix
    GLint u_ViewMatrixLocation = glGetUniformLocation(mShaderID,"u_ViewMatrix");
    if(u_ViewMatrixLocation>=0){
        glm::mat4 viewMatrix = g.gCamera.GetViewMatrix();
        glUniformMatrix4fv(u_ViewMatrixLocation,1,GL_FALSE,&viewMatrix[0][0]);
    }else{
        std::cout << "Could not find u_ViewMatrix, maybe a mispelling?\n";
        exit(EXIT_FAILURE);
    }


    // Projection matrix (in perspective) 
    glm::mat4 perspective = glm::perspective(glm::radians(45.0f),
                                             (float)g.gScreenWidth/(float)g.gScreenHeight,
                                             0.1f,
                                             1000.0f);

    // Retrieve our location of our perspective matrix uniform 
    GLint u_ProjectionLocation= glGetUniformLocation( mShaderID,"u_Projection");
    if(u_ProjectionLocation>=0){
        glUniformMatrix4fv(u_ProjectionLocation,1,GL_FALSE,&perspective[0][0]);
    }else{
        std::cout << "Could not find u_Perspective, maybe a mispelling?\n";
        exit(EXIT_FAILURE);
    }

    // Set Camera Position
    glm::vec3 cameraPos = g.gCamera.GetPosition();
    GLint viewPosLoc = glGetUniformLocation(mShaderID, "u_ViewPos");
    if (viewPosLoc >= 0) {
        glUniform3fv(viewPosLoc, 1, &cameraPos[0]);
    } else {
        std::cout << "Could not find u_ViewPos" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    // Setup the Lights 
    GLint loc = glGetUniformLocation( mShaderID,"u_LightPos");
    if(loc >=0){
        glUniform3fv(loc, 1, &g.gLight.mPosition[0]);
    }else{
        std::cout << "Could not find u_LightPos" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Material properties
    glUniform3f(glGetUniformLocation(mShaderID, "u_MaterialAmbient"), 1.0f, 0.5f, 0.31f);
    glUniform3f(glGetUniformLocation(mShaderID, "u_MaterialDiffuse"), 1.0f, 0.5f, 0.31f);
    glUniform3f(glGetUniformLocation(mShaderID, "u_MaterialSpecular"), 0.5f, 0.5f, 0.5f);
    glUniform1f(glGetUniformLocation(mShaderID, "u_MaterialShininess"), 32.0f);

    // Light properties
    glUniform3f(glGetUniformLocation(mShaderID, "u_LightAmbient"), 0.2f, 0.2f, 0.2f);
    glUniform3f(glGetUniformLocation(mShaderID, "u_LightDiffuse"), 0.5f, 0.5f, 0.5f);
    glUniform3f(glGetUniformLocation(mShaderID, "u_LightSpecular"), 1.0f, 1.0f, 1.0f);

    // Attenuation factors
    glUniform1f(glGetUniformLocation(mShaderID, "u_LightConstant"), 1.0f);
    glUniform1f(glGetUniformLocation(mShaderID, "u_LightLinear"), 0.09f);
    glUniform1f(glGetUniformLocation(mShaderID, "u_LightQuadratic"), 0.032f);
}

void Object::Draw() {
    glBindVertexArray(mVAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mIndices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

std::vector<float> Object::GetVertices() const {
    std::cout << "mVertices:" << mVertices.size() << std::endl;
    return mVertices;
}

std::vector<float> Object::GetNormals() const {
    std::cout << "mNormals :" << mNormals.size() << std::endl;
    return mNormals;
}