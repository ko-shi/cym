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
	//parser.parse();
	ByteCode byte_code = {
		ByteCodeFunc{ {
				OpUnion(OpBinaryOp{ IFBinOp::ASSIGN,0 }),
				OpUnion(OpPushValue{ VariableUnit(Int(20)) }),
				OpUnion(OpCall{}),
				OpUnion(OpBinaryOp{ IFBinOp::PLUS,1 }),
				OpUnion(OpPushPreCall{ 1}),
				OpUnion(OpPushVariable{ 0 }),
				OpUnion(OpPushValue{ VariableUnit(Int(40)) }),
				OpUnion(OpCall{}),
				OpUnion(OpPushValue{VariableUnit(Int(60))}),
				OpUnion(OpTerminate{})
		},2 } ,
		ByteCodeFunc{ {
				OpUnion(OpReturnBinaryOp{ IFBinOp::PLUS }),
				OpUnion(OpPushVariable{ 0 }),
				OpUnion(OpPushVariable{ 1 }),
				OpUnion(OpCall{}),
				OpUnion(OpEndOfReturnFunc{})
		},2 }
	};
	CymVM vm(std::move(byte_code));
	vm.run();
	const auto finish = now();
	std::cout << parser.ast_.toStr() << std::endl;
	std::cout << getNs(finish - start) << std::endl;
	return 0;
}