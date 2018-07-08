#ifndef CYM_OPCODE_HPP
#define CYM_OPCODE_HPP

#include<cym/CymBase.hpp>
#include<cym/vm/CymUnits.hpp>

namespace cym {
	using Uint16 = std::uint16_t;

	struct ByteCodeFunc;

	enum class OpCode {
		ASSIGN,
		PLUS,
		PRECALL,
		PUSHVALUE,
		PUSHPRECALL,
		PUSHVALUETHENCALL,
		PUSHPRECALLTHENCALL,
		CALL,
		ENDOFFUNC,
		TERMINATE
	};

	// Calcrating // // Kind like PreCall
	struct OpAssign {// need 1 push
		VariableUnit* dest;
	};
	struct OpPlus {// need 2 push
		VariableUnit* dest;
	};

	// VM operating //

	// PreCall function
	struct OpPreCall {
		ByteCodeFunc *func;
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

	// End of function
	struct OpEndOfFunc {

	};
	// Terminate
	struct OpTerminate {

	};

	struct OpUnion : Variant<OpAssign, OpPlus, OpPreCall, OpPushValue, OpPushPreCall, OpPushValueThenCall, OpPushPreCallThenCall, OpCall, OpEndOfFunc, OpTerminate> {
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
	constexpr auto s2 = sizeof(OpAssign);
}

#endif