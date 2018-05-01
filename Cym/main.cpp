#include<iostream>
#include<chrono>
#include<fstream>

#include<cym/Cym.hpp>

int main() {

	
	using namespace std::string_literals;
	using namespace cym;
	ICode icode;


	std::fstream infix("infix-operators.cymi");
	std::string str;
	while (std::getline(infix, str)) {
		icode.addInfix(toU16String(toU8String(str)));
	}


	std::fstream file("test.cym");

	while (std::getline(file, str)) {
		icode.addLine(toU16String(toU8String(str)));
	}
	icode.compile();
	std::cout << icode.icode_.getJSON();
	
}