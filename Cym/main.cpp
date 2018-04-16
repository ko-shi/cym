#include<iostream>
#include<fstream>
#include"Cym.hpp"
#include"CymNumConverter.hpp"
#include"CymHandStack.hpp"

int main() {

	
	using namespace std::string_literals;
	/*
	std::fstream file("test.cym");

	cym::Cym project;
	std::string str;
	while (std::getline(file, str)) {
		project.addLine(cym::toU16String(str));
	}
	project.compile();


	project.run();
	std::cout << cym::toSJisString(project.showIntermediateCode());
	*/
	cym::TokenKind kind;
	const auto a = cym::getNumKind(u"-114514.1919g",kind);

	
}