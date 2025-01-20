// g++ -std=c++17 vector2.cpp -o prog
#include <iostream>
#include <vector>

int main(int argc, char** argv){

  std:: vector<unsigned char> myVector;
  for (unsigned int i = 0; i < 91; ++i) {
    myVector.push_back(i);
  }

  std:: cout << "(number of elements) myVector.size() = " << myVector.size() << std:: endl;
  std:: cout << "(size of data structure) sizeof(myVector) = " << sizeof(myVector) << std:: endl;
  std:: cout << "(raw array myVector is built on top of) sizeof(myVector.data()) = " << sizeof(myVector.size()) << std::endl;
  std::cout << std::endl;

  int rawArray[100];
  std:: cout << "locally allocated, i.e. stack allocated array) sizeof(rawArray) = " << sizeof(rawArray)<< std:: endl;
  std::cout << std::endl;

  int* heapArray = new int[52];
  std::cout << "heap allocated array, (i.e. pointer to a chunk of memory) sizeof(heapArray) = " << sizeof(heapArray) << std::endl;
	std::cout << std::endl;	

  delete[] heapArray;

  return 0;
}
