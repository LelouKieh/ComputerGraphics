// g++ -std=c++17 class.cpp -o prog
#include <iostream>
#include <vector>

class PPM{
public: 
    PPM(unsigned int width, unsigned int height) {
        pixelData = new unsigned char[width * height * 3];
    }
    ~PPM() {
        delete[] pixelData;
    }
private:
    uint8_t* pixelData;
};

int main(int argc, char** argv){

    PPM myPPM(64, 128);

    return 0;
}
