#include<iostream>
#include<fstream>
#include"Cym.hpp"
#include"CymHandBuffer.hpp"

int main() {

	
	using namespace std::string_literals;
	using namespace std::string_view_literals;
	
	std::fstream file("test.cym");

	cym::Cym project;
	std::string str;
	while (std::getline(file, str)) {
		project.addLine(cym::toU16String(str));
	}
	project.compile();
	project.run();
	std::cout << cym::toSJisString(project.showMemory());
	
}