#include <iostream> // input and output 'stream' library
#include <vector>   // std::vector data structure.


void PrintVector(const std::vector<int>& v) {
	std:: cout << "PrintVector" << std::endl;
	for (int i = 0; i < v.size(); i++){
		std:: cout << v.at(i) << std::endl;
	}
}

void PrintRawArray(int array[], int size) {
	std:: cout << "PrintRawArray" << std::endl;
	for (int i = 0; i < size; i++){
		std:: cout << array[i] << std::endl;
	}
}

int main(){
	
	std:: cout << "Hello world!" << std::endl;

	// Raw Array
	int array[3] = {1, 2, 3};

	// Declare a vector of integers
	std::vector<int> myVector;
	// Add elements to the vector by pushing them back
	myVector.push_back(1);
	myVector.push_back(2);
	myVector.push_back(3);

	PrintVector(myVector);
	PrintRawArray(myVector.data(), myVector.size());

	std:: cout << "myVector[0]: " << myVector[0] << std::endl;
	std:: cout << "myVector[1]: " << myVector[1] << std::endl;
	std:: cout << "myVector[2]: " << myVector[2] << std::endl;
	
	for (int i = 0; i < myVector.size(); i++){
		std:: cout << myVector[i] << std::endl;
	}

	// C++11 range-based for loop
	for (auto e : myVector){
		std:: cout << e << std::endl;
	}

	return 0;
}
