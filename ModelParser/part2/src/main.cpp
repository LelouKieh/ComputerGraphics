#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>

int gScreenWidth = 640;
int gScreenHeight = 640;
SDL_Window* gGraphicsApplicationWindow = nullptr;
SDL_GLContext gOpenGLContext = nullptr;
bool gQuit = false;
// Shader program
GLuint gGraphicsPipelineShaderProgram = 0;

// Global rotation angle
float gRotationAngle = 0.0f;
class OBJ {
public:
    std::vector<GLfloat> vertexData;
    std::vector<GLuint> indexData;
    size_t indexCount = 0;
    size_t vertexCount = 0;

    bool load(const std::string& path) {
        std::ifstream objFile(path);
        if (!objFile.is_open()) {
            std::cerr << "Failed to open OBJ file: " << path << '\n';
            return false;
        }

        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<GLuint> positionIndices;
        std::vector<GLuint> normalIndices;

        std::string line;
        while (std::getline(objFile, line)) {
            // Skip comments and empty lines
            if (line.substr(0, 1) == "#" || line.empty()) {
                continue;
            }
            std::stringstream ss(line);
            std::string prefix;
            ss >> prefix;

            if (prefix == "v") {
                glm::vec3 position;
                ss >> position.x >> position.y >> position.z;
                positions.push_back(position);
            }
            else if (prefix == "vn") {
                glm::vec3 normal;
                ss >> normal.x >> normal.y >> normal.z;
                normals.push_back(normal);
            }
            else if (prefix == "f") {
                std::vector<GLuint> facePositionIndices;
                std::vector<GLuint> faceNormalIndices;
                std::string vertexStr;
                while (ss >> vertexStr) {
                    std::stringstream vertexSS(vertexStr);
                    std::string indexStr;
                    std::getline(vertexSS, indexStr, '/'); // Position index
                    GLuint posIndex = std::stoi(indexStr) - 1; // OBJ indices start at 1

                    // Skip texture coordinate index
                    if (vertexSS.peek() == '/') vertexSS.get();
                    std::getline(vertexSS, indexStr, '/');

                    std::getline(vertexSS, indexStr); // Normal index
                    GLuint normIndex = std::stoi(indexStr) - 1;

                    if (posIndex < 0 || posIndex >= positions.size()) {
                        std::cerr << "Error: Position index out of bounds: " << posIndex << std::endl;
                        return false;
                    }

                    if (normIndex < 0 || normIndex >= normals.size()) {
                        std::cerr << "Error: Normal index out of bounds: " << normIndex << std::endl;
                        return false;
}

                    facePositionIndices.push_back(posIndex);
                    faceNormalIndices.push_back(normIndex);
                }
                // Triangulate faces (assuming convex polygons)
                for (size_t i = 1; i < facePositionIndices.size() - 1; ++i) {
                    positionIndices.push_back(facePositionIndices[0]);
                    positionIndices.push_back(facePositionIndices[i]);
                    positionIndices.push_back(facePositionIndices[i + 1]);

                    normalIndices.push_back(faceNormalIndices[0]);
                    normalIndices.push_back(faceNormalIndices[i]);
                    normalIndices.push_back(faceNormalIndices[i + 1]);
                }
            }
        }
        objFile.close();

        // Create combined vertex data
        struct Vertex {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec3 color; 
        };

        std::vector<Vertex> vertices;
        std::map<std::pair<GLuint, GLuint>, GLuint> uniqueVertices;
        indexData.clear();

        for (size_t i = 0; i < positionIndices.size(); ++i) {
            GLuint posIndex = positionIndices[i];
            GLuint normIndex = normalIndices[i];

            auto key = std::make_pair(posIndex, normIndex);
            if (uniqueVertices.count(key) == 0) {
                Vertex vertex;
                vertex.position = positions[posIndex];
                vertex.normal = normals[normIndex];
                vertex.color = glm::vec3(1.0f, 0.0f, 0.0f); 
                vertices.push_back(vertex);
                uniqueVertices[key] = vertices.size() - 1;
            }
            indexData.push_back(uniqueVertices[key]);
        }

        // Convert vertex data to GLfloat vector
        vertexData.clear();
        for (const auto& vertex : vertices) {
            vertexData.push_back(vertex.position.x);
            vertexData.push_back(vertex.position.y);
            vertexData.push_back(vertex.position.z);
            vertexData.push_back(vertex.normal.x);
            vertexData.push_back(vertex.normal.y);
            vertexData.push_back(vertex.normal.z);
            vertexData.push_back(vertex.color.r);
            vertexData.push_back(vertex.color.g);
            vertexData.push_back(vertex.color.b);
        }

        vertexCount = vertices.size();

        // Store the index count for drawing
        indexCount = indexData.size();

        return true;
    }
};

// Model management
struct Model {
    OBJ objData;
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    GLuint edgeEBO; // New EBO for edges
    size_t indexCount;
    size_t edgeIndexCount;
    size_t vertexCount; // Number of vertices
};

std::vector<Model> gModels;
int gCurrentModelIndex = 0;
size_t gCubeIndexCount = 0;

// Wireframe mode toggle
bool gWireframeMode = false;


// Function to read shader files
std::string readShaderFile(const std::string& filePath) {
    std::ifstream shaderFile(filePath);
    if (!shaderFile.is_open()) {
        std::cerr << "Failed to open shader file: " << filePath << '\n';
        exit(1);
    }
    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    return shaderStream.str();
}

void LoadModels(const std::vector<std::string>& objFilePaths) {
    gModels.clear();

    for (const auto& path : objFilePaths) {
        Model model;
        if (!model.objData.load(path)) {
            std::cerr << "Failed to load OBJ file: " << path << std::endl;
            exit(1);
        }

        model.indexCount = model.objData.indexCount;

        // Generate edge indices
        std::vector<GLuint> edgeIndices;
        for (size_t i = 0; i < model.objData.indexData.size(); i += 3) {
            GLuint idx0 = model.objData.indexData[i];
            GLuint idx1 = model.objData.indexData[i + 1];
            GLuint idx2 = model.objData.indexData[i + 2];

            edgeIndices.push_back(idx0);
            edgeIndices.push_back(idx1);

            edgeIndices.push_back(idx1);
            edgeIndices.push_back(idx2);

            edgeIndices.push_back(idx2);
            edgeIndices.push_back(idx0);
        }
        model.edgeIndexCount = edgeIndices.size();

        // Generate and bind VAO
        glGenVertexArrays(1, &model.VAO);
        glBindVertexArray(model.VAO);

        // Generate and bind VBO
        glGenBuffers(1, &model.VBO);
        glBindBuffer(GL_ARRAY_BUFFER, model.VBO);
        glBufferData(GL_ARRAY_BUFFER, model.objData.vertexData.size() * sizeof(GLfloat),
                     model.objData.vertexData.data(), GL_STATIC_DRAW);

        // Generate and bind EBO for filled model
        glGenBuffers(1, &model.EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.objData.indexData.size() * sizeof(GLuint),
                     model.objData.indexData.data(), GL_STATIC_DRAW);

        // Generate and bind EBO for edges
        glGenBuffers(1, &model.edgeEBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.edgeEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, edgeIndices.size() * sizeof(GLuint),
                     edgeIndices.data(), GL_STATIC_DRAW);

        GLsizei stride = 9 * sizeof(GLfloat); // 9 floats per vertex

        // Position attribute (location = 0)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

        // Color attribute (location = 1)
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));

        // Normal attribute (location = 2)
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(GLfloat)));

        // Unbind VAO
        glBindVertexArray(0);

        model.vertexCount = model.objData.vertexCount;

        gModels.push_back(model);
    }

    if (gModels.empty()) {
        std::cerr << "No models loaded." << std::endl;
        exit(1);
    }

    // Set initial model index and index count
    gCurrentModelIndex = 0;
    gCubeIndexCount = gModels[gCurrentModelIndex].indexCount;
}


GLuint CompileShader(GLuint type, const char* source) {
    GLuint shaderObject = glCreateShader(type);

    glShaderSource(shaderObject, 1, &source, nullptr);
    glCompileShader(shaderObject);

    // Check for compilation errors
    GLint success;
    glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shaderObject, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    return shaderObject;
}

GLuint CreateShaderProgram(const std::string& vertexShaderSource,
                           const std::string& fragmentShaderSource) {
    GLuint programObject = glCreateProgram();
    GLuint myVertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource.c_str());
    GLuint myFragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource.c_str());
    glAttachShader(programObject, myVertexShader);
    glAttachShader(programObject, myFragmentShader);
    glLinkProgram(programObject);

    // Check for linking errors
    GLint success;
    glGetProgramiv(programObject, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(programObject, 512, NULL, infoLog);
        std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(myVertexShader);
    glDeleteShader(myFragmentShader);

    return programObject;
}

void CreateGraphicsPipeline() {
    std::string vertexShaderSource = readShaderFile("shaders/vert.glsl");
    std::string fragmentShaderSource = readShaderFile("shaders/frag.glsl");

    gGraphicsPipelineShaderProgram = CreateShaderProgram(
        vertexShaderSource,
        fragmentShaderSource);
}

void GetOpenGLVersionInfo() {
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Shading Language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}

void InitializeProgram() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL2 could not initialize video subsystem" << std::endl;
        exit(1);
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    gGraphicsApplicationWindow = SDL_CreateWindow("OpenGL Window",
        100, 100,
        gScreenWidth, gScreenHeight,
        SDL_WINDOW_OPENGL);
    if (gGraphicsApplicationWindow == nullptr) {
        std::cout << "SDL Window was not able to be created" << std::endl;
    }
    gOpenGLContext = SDL_GL_CreateContext(gGraphicsApplicationWindow);
    if (gOpenGLContext == nullptr) {
        std::cout << "OpenGL context not available" << std::endl;
        exit(1);
    }

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cout << "Glad was not initialized" << std::endl;
        exit(1);
    }
    GetOpenGLVersionInfo();
}

void Input() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            std::cout << "Goodbye! (Leaving MainApplicationLoop())" << std::endl;
            gQuit = true;
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_ESCAPE:
                    std::cout << "ESC: Goodbye! (Leaving MainApplicationLoop())" << std::endl;
                    gQuit = true;
                    break;
                case SDLK_q:
                    std::cout << "Q: Goodbye! (Leaving MainApplicationLoop())" << std::endl;
                    gQuit = true;
                    break;
                case SDLK_TAB:
                    gWireframeMode = !gWireframeMode;
                    break;
                case SDLK_w:
                    gWireframeMode = !gWireframeMode;
                    break;
                default:
                    // Check if a number key from 1 to 9 was pressed
                    if ((e.key.keysym.sym >= SDLK_1 && e.key.keysym.sym <= SDLK_9)) {
                        int index = e.key.keysym.sym - SDLK_1;
                        if (index < gModels.size()) {
                            gCurrentModelIndex = index;
                        }
                    }
                    break;
            }
        }
    }
}

void PreDraw() {
    // Calculate rotation angle based on time
    static Uint32 startTime = SDL_GetTicks();
    Uint32 currentTime = SDL_GetTicks();
    float timeElapsed = (currentTime - startTime) / 1000.0f; // Time in seconds

    // Rotation angle
    float rotationSpeed = 10.0f; // degrees per second
    float angle = glm::radians(rotationSpeed * timeElapsed);

    // Create model matrix with rotation and scaling
    glm::mat4 model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
    float scale = 1.0f; // Adjust this value to scale the model
    model = glm::scale(model, glm::vec3(scale, scale, scale));

    // Create view matrix (camera)
    glm::mat4 view = glm::translate(glm::mat4(1.0f),
        glm::vec3(0.0f, 0.0f, -5.0f));

    // Create projection matrix (perspective)
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
        (float)gScreenWidth / (float)gScreenHeight,
        0.1f, 100.0f);

    // Clear the screen and use the shader program
    glViewport(0, 0, gScreenWidth, gScreenHeight);
    glClearColor(0.1f, 0.1f, 0.1f, 1.f);  // Dark background
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glUseProgram(gGraphicsPipelineShaderProgram);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Get the location of the uniform variables
    GLuint modelLoc = glGetUniformLocation(gGraphicsPipelineShaderProgram, "u_ModelMatrix");
    GLuint viewLoc = glGetUniformLocation(gGraphicsPipelineShaderProgram, "u_ViewMatrix");
    GLuint projLoc = glGetUniformLocation(gGraphicsPipelineShaderProgram, "u_Projection");

    // Set the uniform matrices
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void Draw() {
    Model& currentModel = gModels[gCurrentModelIndex];

    // Bind the VAO of the current model
    glBindVertexArray(currentModel.VAO);

    if (gWireframeMode) {
        // Set point size
        glPointSize(1.0f);
        glLineWidth(1.0f);

        // Draw vertices as points
        glDrawArrays(GL_POINTS, 0, currentModel.vertexCount);

        // Draw edges
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, currentModel.edgeEBO);
        glDrawElements(GL_LINES, currentModel.edgeIndexCount, GL_UNSIGNED_INT, 0);
    } else {
        // Render filled model
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, currentModel.EBO);
        glDrawElements(GL_TRIANGLES, currentModel.indexCount, GL_UNSIGNED_INT, 0);
    }

    // Unbind VAO
    glBindVertexArray(0);
}

void MainLoop() {
    while (!gQuit) {
        Input();
        PreDraw();
        Draw();
        SDL_GL_SwapWindow(gGraphicsApplicationWindow);
    }
}

void CleanUp() {
    for (auto& model : gModels) {
        glDeleteVertexArrays(1, &model.VAO);
        glDeleteBuffers(1, &model.VBO);
        glDeleteBuffers(1, &model.EBO);
        glDeleteBuffers(1, &model.edgeEBO);
    }
    glDeleteProgram(gGraphicsPipelineShaderProgram);
    SDL_GL_DeleteContext(gOpenGLContext);
    SDL_DestroyWindow(gGraphicsApplicationWindow);
    SDL_Quit();
}

#ifdef _WIN32
int WinMain(int argc, char* argv[])
{
#else
int main(int argc, char* argv[])
{
#endif
    // Check the OBJ file path
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_obj_file>" << std::endl;
        return 1;
    }
    // Store OBJ file paths
    std::vector<std::string> objFilePaths;
    for (int i = 1; i < argc && i <= 9; ++i) {
        objFilePaths.push_back(argv[i]);
    }

    std::string objFilePath = argv[1];
    InitializeProgram();
    LoadModels(objFilePaths);
    CreateGraphicsPipeline();
    MainLoop();
    CleanUp();
    return 0;
}