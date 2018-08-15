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
		struct Object {
			void* ptr;
			Size size;
		};
		union U {
			Array arr;
			Object obj;
			bool b;
			Int i;
			Uint u;
			double d;
		} data;
		VariableUnit() {

		}
		template<class T>
		VariableUnit(T v){
			data.d = *static_cast<double*>(static_cast<void*>(&v));
		}
	};
	enum class BinOp{
		USER,
		ASSIGN,// following are primitive
		PLUS
	};
	struct PrimitiveFunctionRegister {// singleton
		VariableUnit registers[2];
	};
	struct FunctionUnit {
		BinOp binop;
		const ByteCodeFunc *byte_code;
		Vector<VariableUnit> registers;
		Size pushed;
		PrimitiveFunctionRegister primreg;

		// means from whre this func have called.
		// If any function call any function, it must set this paramator.
		// Nullptr means this function's return is void. 
		// All function implements must set this as return value.
		VariableUnit * caller;


		const OpUnion *itr;
		FunctionUnit(const ByteCodeFunc *b, Size s, VariableUnit *c) : binop(BinOp::USER), byte_code(b), registers(s), pushed(0), caller(c) {
		}
		FunctionUnit(BinOp i, VariableUnit *c) : binop(i),pushed(0), caller(c) {

		}
	};
	constexpr auto a = sizeof(FunctionUnit);
}

#endif