#include<iostream>
#include<chrono>
#include<fstream>
#include"Cym.hpp"
#include"CymIntermediateCodeMaker.hpp"
#include"CymNumConverter.hpp"
#include"CymHandStack.hpp"
#include"CymTree.hpp"

int main() {

	
	using namespace std::string_literals;
	using namespace cym;
	
	std::fstream file("test.cym");

	ICode icode;
	std::string str;
	while (std::getline(file, str)) {
		icode.compileLine(toU16String(toU8String(str)));
	}
	std::cout << icode.init_order_.getJSON();
	
}