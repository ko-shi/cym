#ifndef CYM_RESTRICTION_HPP
#define CYM_RESTRICTION_HPP

#include<cym/CymBase.hpp>
#include<variant>
#include<utility>

#ifdef CONST
#	undef CONST
#endif

namespace cym {
	struct Trait {
		Str name;
		Trait(const Trait &) = default;
		Trait(Trait &&) = default;
		Trait() = default;

		Trait(const Str name) : name(name) {

		}
	};
}
#endif