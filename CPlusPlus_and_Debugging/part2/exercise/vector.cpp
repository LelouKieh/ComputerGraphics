// g++ -std=c++17 vector.cpp -o prog
#include <iostream>
#include <vector>

int main(int argc, char** argv){

  std:: vector<unsigned char> myData;

  for (unsigned char i = 65; i < 91; ++i) {
    myData.push_back(i);
  }

  for (unsigned char i = 0; i < myData.size(); ++i) {
    std:: cout << myData[i] << std:: endl;
  }
  return 0;
}
