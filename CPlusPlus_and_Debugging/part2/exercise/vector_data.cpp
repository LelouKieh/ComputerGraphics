// g++ -std=c++17 vector_data.cpp -o prog
#include <iostream>
#include <vector>

int main(int argc, char** argv){

  std:: vector<int> myData;
  for (int i = 0; i < 42; i++) {
    myData.push_back(i);
  }
  
  int* internalArray = myData.data();
  for (int i = 0; i < myData.size(); i++) {
    std:: cout << internalArray[i] << "\n";
  }

  return 0;
}
