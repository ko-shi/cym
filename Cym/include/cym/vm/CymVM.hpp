#ifndef CYM_VM_HPP
#define CYM_VM_HPP

#include<cym/utils/CymTCPair.hpp>
#include<cym/utils/CymHandStack.hpp>
#include<cym/vm/CymOpcode.hpp>

namespace cym {




	class CymVM {
	public:
		Vector<FunctionUnit> stack_;
		const ByteCode code_;

		CymVM(ByteCode && c) : code_(std::move(c)) {

		}
		void run() {
			stack_ = Vector<FunctionUnit>{};
			stack_.reserve(100);
			if (code_.empty() || code_[0].com.empty()) {
				return;
			}

			VariableUnit main_return;
			const auto &main = code_[0];
			stack_.emplace_back(&main,main.size, &main_return);
			auto itr = main.com.data();
			while (114514) {
				const auto com = *itr;
				const auto opcode = static_cast<OpCode>(com.index());
				auto &func = stack_.back();
				const bool is_prim = func.binop != BinOp::USER;
				switch (static_cast<OpCode>(com.index()))
				{
				case OpCode::BINARYOP: {
					const auto opland = com.as<OpCode::BINARYOP>();
					stack_.emplace_back(opland.op, &func.registers[opland.num]);
					break;
				}
				case OpCode::PUSHVALUE:
					if (is_prim) {
						func.primreg.registers[func.pushed++] = com.as<OpCode::PUSHVALUE>().val;
					}
					else {
						func.registers[func.pushed++] = com.as<OpCode::PUSHVALUE>().val;
					}
					break;
				case OpCode::PUSHPRECALL: {
					const auto opland = com.as<OpCode::PUSHPRECALL>();
					if (is_prim) {
						stack_.emplace_back(&code_[opland.func], code_[opland.func].size, &func.primreg.registers[func.pushed++]);
					}
					else {
						stack_.emplace_back(&code_[opland.func], code_[opland.func].size, &func.registers[func.pushed++]);
					}
					break;
				}
				case OpCode::PUSHVARIABLE: {
					const auto opland = com.as<OpCode::PUSHVARIABLE>();
					if (stack_.size() > 0) {
						const auto former = stack_[stack_.size() - 2];
						const auto val = former.binop == BinOp::USER ?
							former.registers[opland.num]
							: former.primreg.registers[opland.num];
						if (is_prim) {
							func.primreg.registers[func.pushed++] = val;

						}
						else {
							func.registers[func.pushed++] = val;
						}
					}
					break;
				}
				case OpCode::PRECALL: {
					const auto opland = com.as<OpCode::PRECALL>();
					stack_.emplace_back(&code_[opland.func], code_[opland.func].size, &func.registers[opland.num]);
					break;
				}
				case OpCode::CALL:
					if (is_prim) {
						const auto regs = func.primreg.registers;
						switch (func.binop) {
						case BinOp::ASSIGN:
							*func.caller = regs[0];
							break;
						case BinOp::PLUS:
							func.caller->data.i = regs[0].data.i + regs[1].data.i;
							break;
						}
						stack_.pop_back();
					}
					else {
						stack_[stack_.size() - 2].itr = itr;
						itr = func.byte_code->com.data() - 1;// -1 because of incremant by loop end
					}
					break;
				case OpCode::RETURNVALUE:
					if (func.caller) {
						*func.caller = com.as<OpCode::RETURNVALUE>().val;
					}
					stack_.pop_back();
					itr = stack_.back().itr;
					break;
				case OpCode::RETURNOBJECT:
					const auto data = func.caller->data.obj;
					// TODO
					break;
				case OpCode::RETURNFUNC: {
					const auto opland = com.as<OpCode::RETURNFUNC>();
					stack_.emplace_back(&code_[opland.func], code_[opland.func].size, func.caller);
					break;
				}
				case OpCode::RETURNBINOP: {
					const auto opland = com.as<OpCode::RETURNBINOP>();
					stack_.emplace_back(opland.op,func.caller);
					break;
				}
				case OpCode::ENDOFRETURNFUNC:
					stack_.pop_back();
					itr = stack_.back().itr;// not -1 because of CALL
					break;
				default:
					return ;
				}
				itr++;
			}
		}
	};

}

#endif
