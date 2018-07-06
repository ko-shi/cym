#ifndef CYM_OPCODE_HPP
#define CYM_OPCODE_HPP

#include<cym/CymBase.hpp>
#include<cym/utils/CymVector.hpp>

namespace cym {
	using Uint16 = std::uint16_t;
	struct Command;

	struct OpPlus {
		Uint16 left,right;
	};
	struct OpCallFunc {

	};

	struct Command {
		using V = Variant<OpPlus, OpCallFunc>;
		V com;
	};
	constexpr auto a = sizeof(Command);
}

#endif