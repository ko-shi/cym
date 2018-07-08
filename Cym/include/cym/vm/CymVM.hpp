#ifndef CYM_VM_HPP
#define CYM_VM_HPP

#include<cym/utils/CymTCPair.hpp>
#include<cym/utils/CymHandStack.hpp>
#include<cym/vm/CymOpcode.hpp>

namespace cym {

	using ByteCode = Vector<ByteCodeFunc>;// [0] is main function



	class CymVM {

		Vector<FunctionUnit> stack_;
		const ByteCode code_;

		CymVM(ByteCode && c) : code_(std::move(c)) {

		}
		void run() {
			if (code_.empty() || code_[0].com.empty()) {
				return;
			}

			VariableUnit main_return;
			const auto &main = code_[0];
			stack_.emplace_back(main.size, &main_return);
			auto itr = main.com.begin();
			bool is_prim = false;
			while (114514) {
				const auto com = *itr;
				auto &func = stack_.back();
				switch (static_cast<OpCode>(com.index()))
				{
				case OpCode::ASSIGN:
					stack_.emplace_back(IfPrimitive::ASSIGN, com.as<OpCode::ASSIGN>().dest);
					break;
				case OpCode::PUSHVALUE:
					if (is_prim) {
						func.primreg.push(com.as<OpCode::PUSHVALUE>().val);
					}
					else {
						func.registers.emplace_back(com.as<OpCode::PUSHVALUE>().val);
					}
					break;
				case OpCode::CALL:
					if (func.iprim != IfPrimitive::USER) {
						switch (func.iprim) {
						case IfPrimitive::ASSIGN:
							
						}
					}
					else {
						func.itr = itr;
						itr = func.byte_code->com.begin();
					}
					break;
				default:
					return ;
				}
			}
		}
	};

}

#endif
