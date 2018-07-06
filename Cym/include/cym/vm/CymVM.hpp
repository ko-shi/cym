#ifndef CYM_VM_HPP
#define CYM_VM_HPP

#include<cym/utils/CymTCPair.hpp>
#include<cym/utils/CymHandStack.hpp>

namespace cym {
	struct VariableUnit {
		struct Array{
			void* ptr;
			Size size;
		};
		union U{
			Array arr;
			bool b;
			Int i;
			Uint u;
			double d;
		} data;
	};
	struct FunctionUnit {
		Vector<VariableUnit> registers;

		// means from whre this func have called.
		// If any function call any function, it must set this paramator.
		// Nullptr means this function's return is void. 
		// All function implements must set this as return value.
		VariableUnit * caller;
	};

	class CymVM {
		Vector<FunctionUnit> stack;


	};

}

#endif
