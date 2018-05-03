#include<iostream>
#include<chrono>
#include<fstream>

#include<cym/Cym.hpp>
int main() {

	const auto now = []() {return std::chrono::high_resolution_clock::now(); };
	const auto getNs = [](auto p) {return std::chrono::duration_cast<std::chrono::nanoseconds>(p).count(); };

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
	const auto start = now();
	icode.compile();
	const auto finish = now();

	std::cout << getNs(finish - start) << std::endl;

	std::cout << icode.icode_.getJSON();
	
}