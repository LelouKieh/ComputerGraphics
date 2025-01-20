#include <iostream>
#include <string>

int main(){

  std:: string fullName = "Xin Qi";
  std:: string firstName = fullName.substr(0, 3);
  std:: string lastName = fullName.substr(4, 7);
  std:: cout << "First name: " << firstName << std:: endl;
  std:: cout << "Last name: " << lastName << std:: endl;

  unsigned int i = 0;
  while (i < fullName.length()) {
    if (fullName.at(i) == ' ') {
      break;
    }
    std:: cout << fullName[i];
    ++i;
  }

	return 0;
}
