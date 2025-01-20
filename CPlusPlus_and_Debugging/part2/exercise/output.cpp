// g++ -std=c++17 output.cpp -o prog
#include <iostream>
#include <fstream>

int main(){

  std::ofstream outFile;

  outFile.open("temp.txt");
  char my_unsigned_char = 97;

  outFile << "Some data" << std:: endl;
  outFile << "P3" << std:: endl;
  outFile << my_unsigned_char << std:: endl;
  outFile << static_cast<int>(my_unsigned_char) << std:: endl;

  outFile.close();

	return 0;
}
