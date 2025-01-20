// g++ -std=c++17 datasize.cpp -o prog
#include <iostream>

int main(){

  std:: cout << "Data size of int in bytes = " << sizeof(int) << std::endl;
  std:: cout << "Data size of char in bytes = " << sizeof(char) << std::endl;
  std:: cout << "Data size of uint8_t in bytes = " << sizeof(uint8_t) << std::endl;
  std:: cout << "Data size of uint16_t in bytes = " << sizeof(uint16_t) << std::endl;
  std:: cout << "Data size of uint32_t in bytes = " << sizeof(uint32_t) << std::endl;
  std:: cout << "Data size of float in bytes = " << sizeof(float) << std::endl;
  std:: cout<< "Data size of double in bytes = " << sizeof(double) << std::endl;
  std:: cout << "Data size of nullptr_t in bytes = " << sizeof(std::nullptr_t) << std::endl;
  std:: cout << "Data size of long in bytes = " << sizeof(long) << std::endl;
  std:: cout << "Data size of long long in bytes = " << sizeof(long long) << std::endl;
  std:: cout << "Data size of u_char in bytes = " << sizeof(u_char) << std::endl;
  std:: cout << "Data size of long int in bytes = " << sizeof(long int) << std::endl;

	return 0;
}
