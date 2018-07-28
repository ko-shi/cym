#include<iostream>
#include<chrono>
#include<fstream>

#include<cym/Cym.hpp>


int main() {

	const auto now = []() {return std::chrono::high_resolution_clock::now(); };
	const auto getNs = [](auto p) {return std::chrono::duration_cast<std::chrono::nanoseconds>(p).count(); };

	using namespace std::string_literals;
	using namespace cym;
	std::fstream source("example/test.cym");
	std::string input;
	Parser parser;
	while (std::getline(source, input)) {
		parser.addCode(toU16String(input));
	}
	const auto start = now();
	parser.parse();
	const auto finish = now();
	std::cout << parser.ast_.toStr() << std::endl;
	std::cout << getNs(finish - start) << std::endl;
	return 0;
}