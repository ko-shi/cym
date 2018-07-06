#ifndef CYM_VM_HPP
#define CYM_VM_HPP

#include<cym/utils/CymHandStack.hpp>

namespace cym {
	struct VariableUnit {
		void* ptr;// also be row value if it is Int
		Size size;
	};
	constexpr auto s = sizeof(VariableUnit);
	struct FunctionUnit {
		Vector<VariableUnit> registers;

	};

	class CymVM {

	};

}

#endif
