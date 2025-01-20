// g++ -std=c++17 memory.cpp -o prog
#include <iostream>
#include <vector>


int main(int argc, char** argv){

  uint8_t* contiguous_chunk_of_memory = new uint8_t[80];
  uint8_t* one_item = new uint8_t;
  std::cout << "sizeof(contiugous...) = " << sizeof(contiguous_chunk_of_memory) << std::endl;
  std::cout << "sizeof(one_item) = " << sizeof(one_item) << std::endl;
  
  delete one_item;
  delete[] contiguous_chunk_of_memory;

  return 0;
}
