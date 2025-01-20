// g++ -std=c++17 iterator.cpp -o prog
#include <iostream>
#include <vector>

int main(){

	std:: vector<unsigned int> myData;
	for (unsigned int i = 65; i < 91; i++) {
		myData.push_back(i);
	}

	std:: vector<unsigned int>:: iterator it = myData.begin();
	std:: vector<unsigned int>:: iterator end = myData.end();

	for (; it != end; it++) {
		std:: cout << *it << std:: endl;
	}

	return 0;
}
