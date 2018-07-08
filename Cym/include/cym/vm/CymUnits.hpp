#ifndef CYM_UNITS_HPP
#define CYM_UNITS_HPP

#include<cym/CymBase.hpp>

namespace cym {
	struct ByteCodeFunc;
	struct OpUnion;
	struct VariableUnit {
		struct Array {
			void* ptr;
			Size size;
		};
		union U {
			Array arr;
			bool b;
			Int i;
			Uint u;
			double d;
		} data;
	};
	enum class IfPrimitive{
		USER,
		ASSIGN,// following are primitive
	};
	struct PrimitiveFunctionRegister {// singleton
		VariableUnit registers[2];
		Size which = 0;
		void push(VariableUnit v) {
			if (which) {
				registers[1] = v;
			}
			else {
				registers[0] = v;
			}
		}
	};
	struct FunctionUnit {
		IfPrimitive iprim;
		ByteCodeFunc *byte_code;
		Vector<VariableUnit> registers;
		PrimitiveFunctionRegister primreg;

		// means from whre this func have called.
		// If any function call any function, it must set this paramator.
		// Nullptr means this function's return is void. 
		// All function implements must set this as return value.
		VariableUnit * caller;


		Vector<OpUnion>::const_iterator itr;
		FunctionUnit(ByteCodeFunc *b, Size s, VariableUnit *c) : iprim(IfPrimitive::USER), byte_code(b), registers(s), caller(c) {

		}
		FunctionUnit(IfPrimitive i, VariableUnit *c) : iprim(i), caller(c) {

		}
	};
	constexpr auto a = sizeof(FunctionUnit::itr);
}

#endif