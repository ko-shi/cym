#ifndef CYM_RESTRICTION_HPP
#define CYM_RESTRICTION_HPP

#include<cym/CymBase.hpp>
#include<variant>
#include<utility>

#ifdef CONST
#	undef CONST
#endif

namespace cym {
	struct ClassDef;

	struct Trait;

	struct Specific {
		ClassDef* cls_id;
	};

	struct SubType {
		Vector<Trait> req;
	};

	struct FuncTrait {
		StrView name;
		std::unique_ptr<Trait> ret;
		Vector<Trait> args;
	};

	struct Trait {
		using T = Variant<Specific, SubType, FuncTrait>;
		T trait;
	};
}
#endif