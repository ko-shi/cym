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
		InstancedType* arg_type;
		Size size()const {
			return 8; // size of Variable::Array
		}
	};
	struct ObjectType {
		Map<StrView, InstancedType*> members;
	};
	struct FunctionType {

	};
}


#endif // !CYM_INSTANCED_TYPE
