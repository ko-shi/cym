#include<iostream>
#include"Cym.hpp"

int main() {
	cym::Cym project;
	project.addLine(u"#value = 10");
	project.compile();
	project.run();
	std::cout << cym::toSJisString(project.showMemory());
}