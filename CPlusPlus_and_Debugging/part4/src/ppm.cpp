#include "PPM.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <cctype>

// Constructor loads a filename with the .ppm extension
PPM::PPM(std::string fileName){
    // std::ios::binary is used to safely open the file in binary mode
    std::ifstream file(fileName, std::ios::binary);
    if (!file) {
        std::cerr << "Can not opening file " << fileName << std::endl;
        return;
    }

    std::string token;
    // Read magic number
    while (file >> token) {
        if (token[0] == '#') {
            // skip comment line
            file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        } else {
            break;
        }
    }

    std::string magicNumber = token;

    // Magic number other than P3 or P6 is not supported
    if (magicNumber != "P3" && magicNumber != "P6") {
        std::cerr << "PPM format error: " << magicNumber << std::endl;
        return;
    }

    // Read width
    int width = 0, height = 0, maxRange = 0;

    while (file >> token) {
        if (token[0] == '#') {
            file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        } else {
            // convert string tokens read from the file into integer values
            width = std::stoi(token);
            break;
        }
    }

    // Read height
    while (file >> token) {
        if (token[0] == '#') {
            file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        } else {
            height = std::stoi(token);
            break;
        }
    }

    // Read maxRange
    while (file >> token) {
        if (token[0] == '#') {
            file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        } else {
            maxRange = std::stoi(token);
            break;
        }
    }

    m_width = width;
    m_height = height;
    // allocate enough space in the vector to hold data for the image
    m_PixelData.resize(width * height * 3);

    if (magicNumber == "P6") {
        // Consume any whitespace or comments before binary data
        file.get();
        // Retrieve the next character without consuming it
        int ch = file.peek();
        while (isspace(ch) || ch == '#') {
            if (ch == '#') {
                file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            } else {
                file.get();
            }
            ch = file.peek();
        }

        // Read raw(binary) data
        size_t numBytes = width * height * 3;
        // Convert to a char* pointer
        file.read(reinterpret_cast<char*>(m_PixelData.data()), numBytes);
        if (file.gcount() != numBytes) {
            std::cerr << "Error occurred when reading pixel data." << std::endl;
            return;
        }
    } else if (magicNumber == "P3") {
        // Read ASCII pixel data
        size_t numPixels = width * height;
        size_t index = 0;
        int r, g, b;

        for (size_t i = 0; i < numPixels; ++i) {
            // Read R
            while (file >> token) {
                if (token[0] == '#') {
                    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    continue;
                } else {
                    r = std::stoi(token);
                    break;
                }
            }
            // Read G
            while (file >> token) {
                if (token[0] == '#') {
                    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    continue;
                } else {
                    g = std::stoi(token);
                    break;
                }
            }
            // Read B
            while (file >> token) {
                if (token[0] == '#') {
                    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    continue;
                } else {
                    b = std::stoi(token);
                    break;
                }
            }
            // Store pixel data
            m_PixelData[index++] = static_cast<uint8_t>(r);
            m_PixelData[index++] = static_cast<uint8_t>(g);
            m_PixelData[index++] = static_cast<uint8_t>(b);
        }
    }
}

// Destructor deletes(delete or delete[]) any memory that has been allocated
PPM::~PPM(){
}

// Saves a PPM Image to a new file.
void PPM::savePPM(std::string outputFileName) const {
    std::ofstream outFile(outputFileName, std::ios::binary);
    if (!outFile) {
        std::cerr << "Error occurred when opening file for writing: " << outputFileName << std::endl;
        return;
    }

    // Write header
    // Save images using the P6 (binary) format because it's more efficient
    outFile << "P6\n";
    outFile << m_width << " " << m_height << "\n";
    outFile << "255\n";

    // Write pixel data
    outFile.write(reinterpret_cast<const char*>(m_PixelData.data()), m_PixelData.size());
}

// Darken halves (integer division by 2) each of the red, green
// and blue color components of all of the pixels
// in the PPM.
void PPM::darken(){
    for (size_t i = 0; i < m_PixelData.size(); ++i) {
        m_PixelData[i] = m_PixelData[i] / 2;
    }
}

// Lighten doubles (integer multiply by 2) each of the red, green
// and blue color components of all of the pixels
// in the PPM.
void PPM::lighten(){
    for (size_t i = 0; i < m_PixelData.size(); ++i) {
        int value = m_PixelData[i] * 2;
        if (value > 255) {
            value = 255;
        }
        m_PixelData[i] = static_cast<uint8_t>(value);
    }
}

// Sets a pixel to a specific R,G,B value 
void PPM::setPixel(int x, int y, uint8_t R, uint8_t G, uint8_t B){
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        // Out of bounds
        return;
    }
    size_t index = (y * m_width + x) * 3;
    m_PixelData[index] = R;
    m_PixelData[index + 1] = G;
    m_PixelData[index + 2] = B;
}
