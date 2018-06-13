#ifndef CYM_RESTRICTION_HPP
#define CYM_RESTRICTION_HPP

#include<cym/CymBase.hpp>
#include<variant>
#include<utility>

#ifdef CONST
#	undef CONST
#endif

namespace cym {
	struct SubType;
	struct Family;
	using Trait = std::variant<SubType, Family>;

	struct SubType {

	};
	struct Family {

	};
}

#endif