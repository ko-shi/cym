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
	/*
	std::fstream file("test.cym");

	ICode icode;
	std::string str;
	while (std::getline(file, str)) {
		icode.compileLine(toU16String(toU8String(str)));
	}
	*/
	ICode i;
	auto r = i.convertToPolishNotation(u"max(1,2) * min(3,4)");

	auto itr = r.data();
	const auto a = i.convertPNToTree(itr, 1);
	std::cout << r.toString<std::string>([](auto s) {return toSJisString(s.second).toString(); }) << std::endl;
	std::cout << a.getJSON() << std::endl;
}