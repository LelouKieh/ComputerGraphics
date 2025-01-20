// g++ -std=c++17 pointers.cpp -o prog
#include <iostream>

int main(){

  int* px = nullptr;
  int x = 7;
  px = &x;
  std:: cout << "px dereferenced is = " << *px << "\n";

	return 0;
}
