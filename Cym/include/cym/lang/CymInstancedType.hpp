#ifndef CYM_INSTANCED_TYPE
#define CYM_INSTANCED_TYPE

#include<cym/CymBase.hpp>
#include<cym/utils/CymTCVector.hpp>

namespace cym {
	struct InstancedType;
	struct VoidType {

	};
	struct IntType{

	};
	struct ArrayType{
		InstancedType *type;
	};
	struct ObjectType{
		Map<StrView, InstancedType*> member_func;// does not influence size() but does operator==()
		Vector<Pair<StrView, InstancedType*>> members;
	};
	struct FunctionType{
		InstancedType* return_type;
		Vector<InstancedType*> args;
	};

	struct SizeOverload {
		Size operator()(const VoidType &t) {
			return 0;
		}
		Size operator()(const IntType &t) {
			return 4;
		}
		Size operator()(const ArrayType &t) {
			return 8;
		}
		Size operator()(const ObjectType &t) {
			Size s = 0;
			for (auto &i : t.members) {
				s += std::visit(SizeOverload(), i.second->type);
			}
			return s;
		}
		Size operator()(const FunctionType &t) {
			return 4;// ByteCode's index
		}
	};
	struct InstancedType {
		using T = Variant<VoidType, IntType, ArrayType, ObjectType, FunctionType>;
		T type;
		Size size()const {
			return std::visit(SizeOverload(), type);
		}
	};



	struct FuncIdentifier {
		Str name;
		InstancedType *type;
		FuncIdentifier() = default;
		FuncIdentifier(FuncIdentifier&&) = default;
		FuncIdentifier(const FuncIdentifier&) = default;

		template<class T>
		FuncIdentifier(const Str n, T && t) : name(n), type{ std::forward<T>(t) } {

		}
		bool operator==(const FuncIdentifier &f) const{
			return name == f.name && type == f.type;
		}
	};
}


#endif // !CYM_INSTANCED_TYPE
