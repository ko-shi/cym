#include<iostream>
#include"Cym.hpp"

int main() {
	using namespace std::string_literals;
	using namespace std::string_view_literals;
	/*
	cym::Cym project;
	project.addLine(u"#value = 10");
	project.compile();
	project.run();
	std::cout << cym::toSJisString(project.showMemory());
	*/
	cym::TokenClass kind;
	const auto a = cym::takeWord(u"3 + sum(1,2,3)"sv, cym::Vector<cym::StrView>{u"+"}, kind);
}