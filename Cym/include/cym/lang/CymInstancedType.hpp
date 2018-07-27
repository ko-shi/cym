#ifndef CYM_INSTANCED_TYPE
#define CYM_INSTANCED_TYPE

#include<cym/CymBase.hpp>
#include<cym/utils/CymTCVector.hpp>

namespace cym {
	struct InstancedType;
	struct IntType {
		Size size()const {
			return 4;
		}
	};
	struct ArrayType {
		std::unique_ptr<InstancedType> arg_type;
		Size size()const {
			return 8; // size of Variable::Array
		}
	};
	struct ObjectType {
		Map<StrView,InstancedType> mem_func
	};
}


#endif // !CYM_INSTANCED_TYPE
