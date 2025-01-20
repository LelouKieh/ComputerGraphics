#include "Object.hpp"
#include "globals.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <glm/gtc/matrix_transform.hpp> 

// Helper functions to load shaders (provided in the main code)
extern std::string LoadShaderAsString(const std::string& filename);
extern GLuint CreateShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource);

/**
 * @struct VertexKey
 * @brief A struct used to uniquely identify a vertex based on its position, texture, and normal indices.
 */
struct VertexKey {
    unsigned int posIndex;
    unsigned int texIndex;
    unsigned int normIndex;

    /**
     * @brief Overloads the less-than operator to enable VertexKey to be used as a key in std::map.
     * @param other The other VertexKey to compare with.
     * @return True if this VertexKey is less than the other.
     */
    bool operator<(const VertexKey& other) const {
        return std::tie(posIndex, texIndex, normIndex) < std::tie(other.posIndex, other.texIndex, other.normIndex);
    }
};


/**
 * @brief Constructs an Object by parsing the given OBJ file.
 * @param filepath The path to the OBJ file to load.
 */
Object::Object(const std::string& filepath)
    : mFilepath(filepath)
{
    // Extract directory from filepath
    size_t lastSlash = filepath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        mDirectory = filepath.substr(0, lastSlash + 1);
    } else {
        mDirectory = "";
    }
    parseOBJ(filepath);
}


/**
 * @brief Destructor for the Object class. Cleans up allocated OpenGL resources.
 */
Object::~Object()
{
    // Delete OpenGL buffers
    glDeleteBuffers(1, &mVBO_Vertices);
    glDeleteBuffers(1, &mVBO_TexCoords);
    glDeleteBuffers(1, &mVBO_Normals);
    glDeleteBuffers(1, &mEBO);
    glDeleteVertexArrays(1, &mVAO);
    glDeleteProgram(g.gGraphicsPipelineShaderProgram);
}


/**
 * @brief Parses an OBJ file and loads vertex, texture coordinate, and normal data.
 * @param filepath The path to the OBJ file to parse.
 */
void Object::parseOBJ(const std::string& filepath)
{
    std::ifstream objFile(filepath);
    if (!objFile.is_open()) {
        std::cerr << "Failed to open OBJ file: " << filepath << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string line;
    std::string mtlFilename;

    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_texcoords;
    std::vector<glm::vec3> temp_normals;

    std::map<VertexKey, unsigned int> vertexMap;

    while (std::getline(objFile, line)) {
        std::stringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "mtllib") {
            ss >> mtlFilename;
            parseMTL(mDirectory + mtlFilename);
        } else if (prefix == "v") {
            glm::vec3 vertex;
            ss >> vertex.x >> vertex.y >> vertex.z;
            temp_vertices.push_back(vertex);
        } else if (prefix == "vt") {
            glm::vec2 texcoord;
            ss >> texcoord.x >> texcoord.y;
            temp_texcoords.push_back(texcoord);
        } else if (prefix == "vn") {
            glm::vec3 normal;
            ss >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        } else if (prefix == "f") {
            std::vector<unsigned int> faceVertexIndices;
            std::string vertexData;
            while (ss >> vertexData) {
                std::stringstream vertexStream(vertexData);
                std::string value;
                unsigned int posIndex = 0, texIndex = 0, normIndex = 0;
                int index = 0;
                
                while (std::getline(vertexStream, value, '/')) {
                    if (!value.empty()) {
                        unsigned int idx = std::stoi(value);
                        if (index == 0) {
                            posIndex = idx;
                        } else if (index == 1) {
                            texIndex = idx;
                        } else if (index == 2) {
                            normIndex = idx;
                        }
                    }
                    index++;
                }

                // Adjust indices (OBJ format starts counting from 1)
                posIndex--;
                texIndex = (texIndex > 0) ? texIndex - 1 : texIndex;
                normIndex = (normIndex > 0) ? normIndex - 1 : normIndex;

                // Create a unique key for the vertex
                VertexKey key = {posIndex, texIndex, normIndex};

                // Check if the vertex already exists
                if (vertexMap.find(key) == vertexMap.end()) {
                    // Add new vertex data
                    mVertices.push_back(temp_vertices[posIndex]);
                    if (texIndex >= 0 && texIndex < temp_texcoords.size())
                        mTexCoords.push_back(temp_texcoords[texIndex]);
                    else
                        mTexCoords.push_back(glm::vec2(0.0f, 0.0f));

                    if (normIndex >= 0 && normIndex < temp_normals.size())
                        mNormals.push_back(temp_normals[normIndex]);
                    else
                        mNormals.push_back(glm::vec3(0.0f, 0.0f, 0.0f));

                    // Add to map
                    unsigned int newIndex = static_cast<unsigned int>(mVertices.size() - 1);
                    vertexMap[key] = newIndex;
                    faceVertexIndices.push_back(newIndex);
                } else {
                    // Use existing vertex
                    faceVertexIndices.push_back(vertexMap[key]);
                }
            }

            if (faceVertexIndices.size() == 3) {
                mIndices.insert(mIndices.end(), faceVertexIndices.begin(), faceVertexIndices.end());
            } else if (faceVertexIndices.size() > 3) {
                // Triangulate the face
                for (size_t i = 1; i + 1 < faceVertexIndices.size(); ++i) {
                    mIndices.push_back(faceVertexIndices[0]);
                    mIndices.push_back(faceVertexIndices[i]);
                    mIndices.push_back(faceVertexIndices[i + 1]);
                }
            } else {
                std::cerr << "Face with less than 3 vertices encountered.\n";
            }
        }
    }
}

/**
 * @brief Parses an MTL file to load material properties, specifically the diffuse texture map.
 * @param filepath The path to the MTL file to parse.
 */
void Object::parseMTL(const std::string& filepath)
{
    std::ifstream mtlFile(filepath);
    if (!mtlFile.is_open()) {
        std::cerr << "Failed to open MTL file: " << filepath << std::endl;
        return;
    }

    std::string line;
    std::string prefix;
    while (std::getline(mtlFile, line)) {
        std::stringstream ss(line);
        ss >> prefix;
        if (prefix == "map_Kd") {
            // Texture map
            ss >> mTextureFilepath;
            // Append directory if necessary
            mTextureFilepath = mDirectory + mTextureFilepath;
            std::cout << "Texture file found: " << mTextureFilepath << std::endl;
        }
    }
}

/**
 * @brief Initializes the object by setting up shaders, buffers, and loading the texture.
 */
void Object::Initialize()
{
    // Load the texture
    mPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    if (!mTextureFilepath.empty()) {
        mTexture.LoadTexture(mTextureFilepath);
    } else {
        std::cerr << "No texture file specified in MTL file.\n";
        exit(EXIT_FAILURE);
    }

    // Create shaders
    std::string vertexShaderSource = LoadShaderAsString("./shaders/vert.glsl");
    std::string fragmentShaderSource = LoadShaderAsString("./shaders/frag.glsl");
    g.gGraphicsPipelineShaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // Create buffers
    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);

    // VBO for vertices
    glGenBuffers(1, &mVBO_Vertices);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO_Vertices);
    glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(glm::vec3), &mVertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // VBO for texture coordinates
    glGenBuffers(1, &mVBO_TexCoords);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO_TexCoords);
    glBufferData(GL_ARRAY_BUFFER, mTexCoords.size() * sizeof(glm::vec2), &mTexCoords[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // VBO for normals
    glGenBuffers(1, &mVBO_Normals);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO_Normals);
    glBufferData(GL_ARRAY_BUFFER, mNormals.size() * sizeof(glm::vec3), &mNormals[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // EBO for indices
    glGenBuffers(1, &mEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * sizeof(unsigned int), &mIndices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);
    std::cout << "Number of vertices loaded: " << mVertices.size() << std::endl;
    std::cout << "Number of indices loaded: " << mIndices.size() << std::endl;
    std::cout << "Number of texture coordinates loaded: " << mTexCoords.size() << std::endl;
    std::cout << "Number of normals loaded: " << mNormals.size() << std::endl;
}

/**
 * @brief Prepares the object for drawing by setting uniforms and binding textures.
 */
void Object::PreDraw()
{
    // Use shader 
    glUseProgram(g.gGraphicsPipelineShaderProgram);

    // Model transformation by translating our object into world space
    glm::mat4 model = glm::translate(glm::mat4(1.0f),glm::vec3(0.0f,0.0f,g.g_uOffset)); 
    model = glm::rotate(model,glm::radians(g.g_uRotate),glm::vec3(0.0f,1.0f,0.0f));
    
    // auto rotate
    static float rot=0.0f;
    rot += 0.01f;
    model = glm::rotate(model,glm::radians(rot),glm::vec3(0.0f,1.0f,0.0f)); 

    // Retrieve our location of our Model Matrix
    GLint u_ModelMatrixLocation = glGetUniformLocation(g.gGraphicsPipelineShaderProgram, "u_ModelMatrix");
    if (u_ModelMatrixLocation >= 0) {
        glUniformMatrix4fv(u_ModelMatrixLocation, 1, GL_FALSE, &model[0][0]);
    } else {
        std::cout << "Could not find u_ModelMatrix, maybe a misspelling?\n";
        exit(EXIT_FAILURE);
    }

    // Update the View Matrix
    GLint u_ViewMatrixLocation = glGetUniformLocation(g.gGraphicsPipelineShaderProgram, "u_ViewMatrix");
    if (u_ViewMatrixLocation >= 0) {
        glm::mat4 viewMatrix = g.gCamera.GetViewMatrix();
        glUniformMatrix4fv(u_ViewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
    } else {
        std::cout << "Could not find u_ViewMatrix, maybe a misspelling?\n";
        exit(EXIT_FAILURE);
    }

    // Projection matrix (in perspective) 
    glm::mat4 perspective = glm::perspective(glm::radians(45.0f),
                                             (float)g.gScreenWidth / (float)g.gScreenHeight,
                                             0.1f,
                                             100.0f);

    // Retrieve our location of our perspective matrix uniform 
    GLint u_ProjectionLocation = glGetUniformLocation(g.gGraphicsPipelineShaderProgram, "u_Projection");
    if (u_ProjectionLocation >= 0) {
        glUniformMatrix4fv(u_ProjectionLocation, 1, GL_FALSE, &perspective[0][0]);
    } else {
        std::cout << "Could not find u_Projection, maybe a misspelling?\n";
        exit(EXIT_FAILURE);
    }

    // Bind texture
    mTexture.Bind(0);

    // Setup our uniform for our texture
    GLint u_textureLocation = glGetUniformLocation(g.gGraphicsPipelineShaderProgram, "u_DiffuseTexture");
    if (u_textureLocation >= 0) {
        // Setup the slot for the texture
        glUniform1i(u_textureLocation, 0);
    } else {
        std::cout << "Could not find u_DiffuseTexture, maybe a misspelling?" << std::endl;
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Draws the object by binding the VAO and issuing a draw call.
 */
void Object::Draw()
{
    glBindVertexArray(mVAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mIndices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}
