// g++ -std=c++17 input.cpp -o prog
#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char** argv){

  std:: ifstream inFile;
  if (argc != 2) {
    std:: cerr<< "Two arguments needed\n" << std:: endl;
    std:: cerr<< "ProgramName file_path.\n" << std:: endl;
    return 0;
  }

  inFile.open(argv[1]);
  if (inFile.is_open()) {
    std::string line;
    while(std:: getline(inFile, line)) {
      std:: cout << line << std:: endl;
    }
  } else {
    std:: cout << "Filepath doesn't exist" << argv[1] << " could not be opened." << std:: endl;
  }

  inFile.close();

	return 0;
}
