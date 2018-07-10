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
			while (114514) {
				const auto com = *itr;
				auto &func = stack_.back();
				const bool is_prim = func.iprim != IfPrimitive::USER;
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
				case OpCode::PUSHPRECALL: {
					const auto opland = com.as<OpCode::PUSHPRECALL>();
					func.registers.emplace_back();
					stack_.emplace_back(opland.func, opland.func->size, &func.registers.back().data);
					break;
				}
				case OpCode::PRECALL: {
					const auto opland = com.as<OpCode::PRECALL>();
					stack_.emplace_back(opland.func, opland.func->size, opland.caller);
					break;
				}
				case OpCode::CALL:
					if (is_prim) {
						switch (func.iprim) {
						case IfPrimitive::ASSIGN:
							*func.caller = func.primreg.registers[0];
							break;
						}
					}
					else {
						func.itr = itr;
						itr = func.byte_code->com.begin();
					}
					break;
				case OpCode::RETURNVALUE:
					*func.caller = com.as<OpCode::RETURNVALUE>().val;
					stack_.pop_back();
					break;
				case OpCode::RETURNOBJECT:
					const auto data = func.caller->data.obj;
					// TODO
					break;
				case OpCode::RETURNFUNC: {
					const auto opland = com.as<OpCode::RETURNFUNC>();
					stack_.emplace_back(opland.func, opland.func->size, func.caller);
					break;
				}
				case OpCode::ENDOFRETURNFUNC:
					stack_.pop_back();
					break;
				default:
					return ;
				}
			}
		}
	};

}

#endif
