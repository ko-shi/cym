#ifndef CYM_OPCODE_HPP
#define CYM_OPCODE_HPP

#include<cym/CymBase.hpp>
#include<cym/vm/CymUnits.hpp>

namespace cym {
	using Uint16 = std::uint16_t;

	struct ByteCodeFunc;


	// Calcrating // // Kind like PreCall
	struct OpBinaryOp {
		IFBinOp op;
		Uint num;
	};

	// VM operating //


	// PreCall function
	struct OpPreCall {
		ByteCodeFunc *func;
		Uint num;
	};

	// Push
	struct OpPushValue {
		VariableUnit val;
	};
	struct OpPushPreCall {
		ByteCodeFunc *func;
	};
	// Push then call
	struct OpPushValueThenCall {
		VariableUnit val;
	};
	struct OpPushPreCallThenCall {
		ByteCodeFunc *func;
	};

	// Call function
	struct OpCall {// with exiting
		
	};

	// Return, end of function
	struct OpReturnValue {
		VariableUnit val;
	};
	struct OpReturnObject {
		VariableUnit::Object obj;
	};
	// Kind like a precall, caller is func.caller
	struct OpReturnFunc {
		ByteCodeFunc* func;
	};
	struct OpEndOfReturnFunc {

	};

	// Terminate
	struct OpTerminate {

	};

	enum class OpCode {
		BINARYOP,
		PRECALL,
		PUSHVALUE,
		PUSHPRECALL,
		PUSHVALUETHENCALL,
		PUSHPRECALLTHENCALL,
		CALL,
		RETURNVALUE,
		RETURNOBJECT,
		RETURNFUNC,
		ENDOFRETURNFUNC,
		TERMINATE
	};

	using OpBase = Variant<
		OpBinaryOp,
		OpPreCall,
		OpPushValue,
		OpPushPreCall,
		OpPushValueThenCall,
		OpPushPreCallThenCall,
		OpCall,
		OpReturnValue,
		OpReturnObject,
		OpReturnFunc,
		OpEndOfReturnFunc,
		OpTerminate
	>;
	struct OpUnion : OpBase{
		template<class T>
		OpUnion(T &&arg) : OpBase(std::forward<T>(arg)) {

		}
		template<OpCode C>
		auto as() const {
			return std::get<static_cast<Size>(C)>(*this);
		}
	};

	struct ByteCodeFunc {
		Vector<OpUnion> com;
		Size size;// num of variables
	};

	constexpr auto s = sizeof(OpUnion);
	constexpr auto s2 = sizeof(OpPushValue);
}

#endif