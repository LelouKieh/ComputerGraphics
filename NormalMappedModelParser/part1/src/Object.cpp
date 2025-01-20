#include "Object.hpp"
#include "globals.hpp"
#include "Light.hpp"
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
 * @brief Constructs an Object by loading data from an OBJ file.
 *
 * The constructor initializes the object by loading vertex, texture, and normal data 
 * from the specified OBJ file. It also extracts the directory from the filepath 
 * for locating related material files (MTL).
 *
 * @param filepath Path to the OBJ file to load.
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
    glDeleteBuffers(1, &mVBO_Tangents);
    glDeleteBuffers(1, &mVBO_Bitangents);
    glDeleteProgram(g.gGraphicsPipelineShaderProgram);
}


/**
 * @brief Parses an OBJ file to load vertex, texture, and normal data for rendering.
 *
 * This function reads an OBJ file line by line, extracting vertex positions, 
 * texture coordinates, and normals. It uses a map to avoid duplicate vertices 
 * by creating unique keys for each vertex. Indices are stored for indexed drawing,
 * and faces are triangulated if they have more than 3 vertices.
 *
 * @param filepath Path to the OBJ file.
 * @throws runtime_error if the OBJ file cannot be opened.
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
    objFile.close();
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
            mTexture.LoadTexture(mTextureFilepath);
        } else if (prefix == "map_Bump") {
            // Normal map
            std::string normalMapFilepath;
            ss >> normalMapFilepath;
            // Append directory if necessary
            normalMapFilepath = mDirectory + normalMapFilepath;
            std::cout << "Normal map file found: " << normalMapFilepath << std::endl;
            mNormalMapTexture.LoadTexture(normalMapFilepath);
        }
    }
    mtlFile.close();
}

/**
 * @brief Initializes the object by setting up shaders, buffers, and loading the texture.
 */
void Object::Initialize()
{
    // Create shaders
    std::string vertexShaderSource = LoadShaderAsString("./shaders/vert.glsl");
    std::string fragmentShaderSource = LoadShaderAsString("./shaders/frag.glsl");
    g.gGraphicsPipelineShaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);

    ComputeTangentSpace();

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

    // VBO for tangents
    GLuint mVBO_Tangents;
    glGenBuffers(1, &mVBO_Tangents);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO_Tangents);
    glBufferData(GL_ARRAY_BUFFER, mTangents.size() * sizeof(glm::vec3), &mTangents[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(3); // Location 3 in shader
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // VBO for bitangents
    GLuint mVBO_Bitangents;
    glGenBuffers(1, &mVBO_Bitangents);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO_Bitangents);
    glBufferData(GL_ARRAY_BUFFER, mBitangents.size() * sizeof(glm::vec3), &mBitangents[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(4); // Location 4 in shader
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindVertexArray(0);
    std::cout << "Number of vertices loaded: " << mVertices.size() << std::endl;
    std::cout << "Number of indices loaded: " << mIndices.size() << std::endl;
    std::cout << "Number of texture coordinates loaded: " << mTexCoords.size() << std::endl;
    std::cout << "Number of normals loaded: " << mNormals.size() << std::endl;
}


/**
 * @brief Prepares the object for drawing by setting uniforms and binding textures.
 *
 * This function applies model transformations (translation and rotation), sets 
 * various uniform variables (model, view, and projection matrices), binds the
 * texture and normal map, and passes light and view positions for lighting.
 *
 * @return void
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
    // rot += 0.01f;
    model = glm::rotate(model,glm::radians(rot),glm::vec3(0.0f,1.0f,0.0f)); 

    glm::vec3 lightPos = glm::vec3(0.0f, 10.0f, 10.0f);

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

    // Set light position uniform
    GLint u_LightPosLocation = glGetUniformLocation(g.gGraphicsPipelineShaderProgram, "u_LightPos");
    if (u_LightPosLocation >= 0) {
        glm::vec3 lightPos = g.gLight.GetPosition();
        glUniform3f(u_LightPosLocation, lightPos.x, lightPos.y, lightPos.z);
    }

    // Set view position uniform
    GLint u_ViewPosLocation = glGetUniformLocation(g.gGraphicsPipelineShaderProgram, "u_ViewPos");
    if (u_ViewPosLocation >= 0) {
        glUniform3f(u_ViewPosLocation, g.gCamera.GetEyeXPosition(), g.gCamera.GetEyeYPosition(), g.gCamera.GetEyeZPosition());
    }

    // Bind the normal map texture
    mNormalMapTexture.Bind(1); // Bind to texture unit 1

    // Set the normal map sampler uniform
    GLint u_NormalMapLocation = glGetUniformLocation(g.gGraphicsPipelineShaderProgram, "u_NormalMap");
    if (u_NormalMapLocation >= 0) {
        glUniform1i(u_NormalMapLocation, 1); // Texture unit 1
    }
}


/**
 * @brief Draws the object by binding the VAO and issuing a draw call.
 *
 * This function binds the VAO associated with the object and renders it 
 * using the indices previously stored in the EBO, then unbinds the VAO.
 *
 * @return void
 */
void Object::Draw()
{
    glBindVertexArray(mVAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mIndices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}


/**
 * @brief Computes tangent and bitangent vectors for each vertex to support normal mapping.
 *
 * This function calculates tangents and bitangents for each triangle in the mesh 
 * based on the differences in positions and texture coordinates. These values are 
 * accumulated per vertex to allow smooth normal mapping and are then normalized.
 * 
 * @return void
 */
void Object::ComputeTangentSpace()
{
    mTangents.resize(mVertices.size(), glm::vec3(0.0f));
    mBitangents.resize(mVertices.size(), glm::vec3(0.0f));

    // Iterate over each triangle
    for (size_t i = 0; i < mIndices.size(); i += 3)
    {
        // Get the indices of the triangle vertices
        unsigned int i0 = mIndices[i];
        unsigned int i1 = mIndices[i + 1];
        unsigned int i2 = mIndices[i + 2];

        // Get the vertex positions
        glm::vec3& v0 = mVertices[i0];
        glm::vec3& v1 = mVertices[i1];
        glm::vec3& v2 = mVertices[i2];

        // Get the texture coordinates
        glm::vec2& uv0 = mTexCoords[i0];
        glm::vec2& uv1 = mTexCoords[i1];
        glm::vec2& uv2 = mTexCoords[i2];

        // Calculate the edges of the triangle
        glm::vec3 deltaPos1 = v1 - v0;
        glm::vec3 deltaPos2 = v2 - v0;

        // Calculate the differences in UV coordinates
        glm::vec2 deltaUV1 = uv1 - uv0;
        glm::vec2 deltaUV2 = uv2 - uv0;

        float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
        glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
        glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

        // Accumulate the tangents and bitangents
        mTangents[i0] += tangent;
        mTangents[i1] += tangent;
        mTangents[i2] += tangent;

        mBitangents[i0] += bitangent;
        mBitangents[i1] += bitangent;
        mBitangents[i2] += bitangent;
    }

    // Normalize the tangents and bitangents
    for (size_t i = 0; i < mVertices.size(); ++i)
    {
        mTangents[i] = glm::normalize(mTangents[i]);
        mBitangents[i] = glm::normalize(mBitangents[i]);
    }
}
