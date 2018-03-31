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
	const auto a = cym::convertToRPN(u"3 + sum(1,2,3)"sv, cym::Vector<cym::StrView>{u"(", u")", u"+"}, [](const cym::StrView &l, const cym::StrView &r) {
		static const std::unordered_map<cym::StrView, int> priority = { { u"(",2 },{ u")",2 },{ u"=",16 },{ u"+",6 },{ u"-",6 },{ u"*",5 },{ u"/",5 } };
		return priority.at(l) > priority.at(r);
	});
	std::cout << cym::toSJisString(cym::showRPN<cym::Str>(a));
}