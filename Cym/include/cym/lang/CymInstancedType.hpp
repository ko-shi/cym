#ifndef CYM_INSTANCED_TYPE
#define CYM_INSTANCED_TYPE

#include<cym/CymBase.hpp>
#include<cym/utils/CymVector.hpp>

namespace cym {

	enum class TypeId {
		INT,ARRAY,STRUCT,FUNC
	};
	struct TypeBase {
		virtual TypeId id()const = 0;
		virtual bool compare(const TypeBase* a)const {
			return id() == a->id();
		}
		virtual ~TypeBase() = 0;
	};
	struct IntType : TypeBase {
		virtual TypeId id()const override {
			return TypeId::INT;
		}
	};
	struct ArrayType : TypeBase {
		std::unique_ptr<TypeBase> elem;
		virtual bool compare(const TypeBase* a)const override {
			if (a->id() == TypeId::ARRAY) {
				return dynamic_cast<const ArrayType*>(a)->elem->compare(elem.get());
			}
			return false;
		}
		virtual TypeId id()const override {
			return TypeId::ARRAY;
		}
	};
	struct StructType : TypeBase {
		Map<Str, std::unique_ptr<TypeBase>> member;
		virtual bool compare(const TypeBase* a)const override {
			if (a->id() == TypeId::STRUCT) {
				const auto struct_a = dynamic_cast<const StructType*>(a);
				if (member.size() != struct_a->member.size()) {
					return false;
				}
				for (const auto &i : struct_a->member) {
					const auto itr = member.find(i.first);
					if (itr == member.end()) {
						return false;
					}
					if (!itr->second->compare(i.second.get())) {
						break;
					}
				}
				return true;
			}
			return false;
		}
		virtual TypeId id()const override {
			return TypeId::STRUCT;
		}
	};


	struct FuncType : TypeBase {
		std::unique_ptr<TypeBase> ret_type;
		Vector<std::unique_ptr<TypeBase>> arg_type;
		virtual bool compare(const TypeBase* a)const override {
			if (a->id() == TypeId::STRUCT) {
				const auto struct_a = dynamic_cast<const FuncType*>(a);
				if (arg_type.size() != struct_a->arg_type.size()) {
					return false;
				}
				if (!struct_a->ret_type->compare(ret_type.get())) {
					return false;
				}
				for (Size i = 0; i < arg_type.size();i++) {
					if (!arg_type[i]->compare(struct_a->arg_type[i].get())) {
						return false;
					}
				}
				return true;
			}
			return false;
		}
		virtual TypeId id()const override {
			return TypeId::FUNC;
		}
	};

	struct TypeComp {
		bool operator()(const std::unique_ptr<TypeBase> &l, const std::unique_ptr<TypeBase> &r) const{
			return l->compare(r.get());
		}
	};
}


#endif // !CYM_INSTANCED_TYPE
