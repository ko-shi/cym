#include<iostream>
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
	Map<StrView, Size> p{ {u"+",1},{u"*",2} };
	ICode i;
	const auto r = i.getCallList(u"114514 * 2 + 810 * 3",p);
	std::cout << r.toString<std::string>([](auto s) {return toSJisString(s.second).toString(); });
}