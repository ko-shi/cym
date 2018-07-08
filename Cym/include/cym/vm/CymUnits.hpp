#ifndef CYM_UNITS_HPP
#define CYM_UNITS_HPP

#include<cym/CymBase.hpp>
#include<cym/utils/CymVector.hpp>

namespace cym {

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
	struct FunctionUnit {
		Vector<VariableUnit> registers;

		// means from whre this func have called.
		// If any function call any function, it must set this paramator.
		// Nullptr means this function's return is void. 
		// All function implements must set this as return value.
		VariableUnit * caller;
	};

}

#endif