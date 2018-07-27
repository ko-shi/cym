#ifndef CYM_RESTRICTION_HPP
#define CYM_RESTRICTION_HPP

#include<cym/CymBase.hpp>
#include<variant>
#include<utility>

#ifdef CONST
#	undef CONST
#endif

namespace cym {

	struct Trait;
	struct FuncTrait;
	struct RefTrait {
		StrView name;// other trait's name
	};

	struct ClassTrait {
		Vector<Pair<Str, Trait>> mem_params;
		Vector<FuncTrait> mem_funcs;

		Vector<ClassTrait> inner_cls;
	};

	struct FuncTrait {
		StrView name;
		std::unique_ptr<Trait> ret;
		Vector<Pair<StrView,Trait>> args;
	};

	struct Trait {
		using T = Variant<RefTrait, ClassTrait, FuncTrait>;
		T trait;
		Trait() = default;
		Trait(const Trait&) = default;
		Trait(Trait &&) = default;
		
		template<class Y>
		Trait(Y &&x) : trait(std::forward<Y>(x)) {

		}
	};
}
#endif