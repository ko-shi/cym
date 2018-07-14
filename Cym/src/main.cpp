#include<iostream>
#include<chrono>
#include<fstream>

#include<cym/Cym.hpp>
int main() {

	const auto now = []() {return std::chrono::high_resolution_clock::now(); };
	const auto getNs = [](auto p) {return std::chrono::duration_cast<std::chrono::nanoseconds>(p).count(); };

	using namespace std::string_literals;
	using namespace cym;
	ByteCode byte_code = { 
		ByteCodeFunc{{
			OpUnion(OpBinaryOp{IFBinOp::ASSIGN,0}),
			OpUnion(OpPushValue{VariableUnit(Int(20))}),
			OpUnion(OpCall{}),
			OpUnion(OpPreCall{1,1}),
			OpUnion(OpPushVariable{0}),
			OpUnion(OpPushValue{VariableUnit(Int(40))}),
			OpUnion(OpCall{}),
			OpUnion(OpTerminate{})
		},2} ,
		ByteCodeFunc{{
			OpUnion(OpReturnBinaryOp{IFBinOp::PLUS}),
			OpUnion(OpPushVariable{0}),
			OpUnion(OpPushVariable{1}),
			OpUnion(OpCall{}),
			OpUnion(OpEndOfReturnFunc{})
		},2}
	};
	CymVM vm(std::move(byte_code));
	vm.run();
	return 0;
}