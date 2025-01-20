/** @file PPM.hpp
 *  @brief Class for working with PPM images
 *  
 *  Class for working with P3 PPM images specifically.
 */
#ifndef PPM_HPP
#define PPM_HPP

#include <string>
#include <vector>
#include <cstdint>

class PPM{
public:
    // Constructor loads a filename with the .ppm extension
    PPM(std::string fileName);
    // Destructor clears any memory that has been allocated
    ~PPM();
    // Saves a PPM Image to a new file.
    void savePPM(std::string outputFileName) const;
    // Darken halves (integer division by 2) each of the red, green
    // and blue color components of all of the pixels
    // in the PPM. Note that no values may be less than
    // 0 in a ppm.
    void darken();
    // Lighten doubles (integer multiply by 2) each of the red, green
    // and blue color components of all of the pixels
    // in the PPM.
    void lighten();
    // Sets a pixel to a specific R,G,B value 
    void setPixel(int x, int y, uint8_t R, uint8_t G, uint8_t B);
    // Returns the raw pixel data in an array.
    inline unsigned char* pixelData() const { return const_cast<unsigned char*>(m_PixelData.data()); }
    // Returns image width
    inline int getWidth() const { return m_width; }
    // Returns image height
    inline int getHeight() const { return m_height; }
private:    
    // Store the raw pixel data here
    // Data is R,G,B format
    std::vector<uint8_t> m_PixelData;
    // Store width and height of image.
    int m_width{0};
    int m_height{0};
};


#endif
