#ifndef CYM_HPP
#define CYM_HPP

#include<string>
#include<vector>
#include<algorithm>

#include"CymVector.hpp"
#include"CymStack.hpp"
#include"CymDoubleKeyMap.hpp"
#include"CymDataTypes.hpp"
#include"CymStringConverter.hpp"

namespace cym {

	struct SSPairHash {
		std::size_t operator()(const std::pair<std::size_t, std::size_t> &p) const{
			return std::hash<std::size_t>()(p.first) ^ std::hash<std::size_t>()(p.second);
		}
	};

	class Cym {
	public:

	private:
		// compile time
		std::vector<Str> code_;
		Vector<WordInfo> word_info_;
		DoubleKeyMap<std::pair<std::size_t, std::size_t>/* line and pos */, ParamIdentifier,SSPairHash> param_identifier_;

		
		DoubleKeyMap<ClassIdentifier, ClassInfo> class_info_;
		DoubleKeyMap<FuncIdentifier, FuncInfo> func_info_;
		std::unordered_map<Str, std::size_t> priorities_;
		Vector<Str> operators_;
		Vector<Str> reserved_words_;

		// runtime
		std::size_t current_scope;
		Stack<FuncInstance> call_stack_;
	public:
		Cym() {
			const std::pair<Str, std::size_t> default_operators[]
				= { { u"(",2 },{ u")",2 },{ u"=",16 },{ u"+",6 },{ u"-",6 },{ u"*",5 },{ u"/",5 } };
			for (const auto &i : default_operators) {
				priorities_.emplace(i);
			}
			operators_ = Vector<Str>{ u"(", u")", u"=", u"+", u"-", u"*", u"/" };

		}
		int addFunc(const Str &scope,const Vector<std::size_t> &args,const Str &name,std::size_t param_num,std::size_t default_size,const Vector<Command> &command) {
			func_info_.emplace(FuncIdentifier{ scope,args,name }, FuncInfo{param_num,default_size,command});
			return 0;
		}
		int addClass(const Str &name_space,const Str &name,std::size_t size,std::size_t param_num,const Vector<std::size_t> params, const Vector<std::size_t> funcs) {
			class_info_.emplace(ClassIdentifier{ name_space,name }, ClassInfo{ size,param_num,params,funcs });
			return 0;
		}
		int addLine(const Str &str) {
			code_.emplace_back(str);
			return 0;
		}
		int compileLine(const Str &str,std::size_t line) {
			TokenClass kind;
			auto scope = Str(u"terminal/Main");
			const auto head = takeWord(str, reserved_words_, operators_, kind);
			switch (kind) {
			case TokenClass::RESERVEDWORD: {
				WordInfo reserved_word_word_info;
				reserved_word_word_info.kind = kind;
				reserved_word_word_info.name = head;
				reserved_word_word_info.place = std::make_pair(line,str.find(head));
				word_info_.emplaceBack(reserved_word_word_info);
				if (head == u"var") {
					const auto var_name = seekToNextWord(str, head, reserved_words_, operators_, kind);
					if (kind != TokenClass::NAME) {
						//TODO コンパイルエラー処理
					}
					const auto name_place = std::make_pair(line, str.find(head));
					param_identifier_.emplace(name_place, ParamIdentifier{scope,});
					WordInfo var_name_word_info{kind,var_name,};

				}
			}
			}
			return 0;
		}
		int compile() {
			std::size_t line = 0;
			for (const auto &i : code_) {
				compileLine(i,line);
				line++;
			}
			addClass(u"terminal", u"Main", 0, 0, Vector<std::size_t>{}, Vector<std::size_t>{0});
			addClass(u"terminal", u"PrimaryInt", 4, 1, Vector<std::size_t>{}, Vector<std::size_t>{});
			addFunc(u"terminal/Main", Vector<std::size_t>{}, u"main", 1, 4, Vector<Command>{
				Command(Command::ALLOC_CLASS, 0, class_info_.indexOf(ClassIdentifier{Str(u"terminal"),Str(u"PrimaryInt")})),
				Command(Command::SUBSTITUTE, 0, 0x01234567)
			});

			return 0;
		}
		int run() {
			const auto main_func_identifier = FuncIdentifier{ Str(u"terminal/Main"),Vector<std::size_t>{},Str(u"main") };
			auto main_func_info = func_info_[main_func_identifier];
			call_stack_.emplace(func_info_.indexOf(main_func_identifier), main_func_info.param_num, main_func_info.default_size);
			
			for (auto com = main_func_info.command.begin(); com != main_func_info.command.end(); com++) {
				executeCommand(com);
			}
			return 0;
		}
		Str showMemory(){
			const auto findFuncIdentifier = [&](std::size_t i) {
				auto func_name = FuncIdentifier{ u"error",Vector<std::size_t>{},u"not found" };
				for (const auto &p : func_info_.link) {
					if (p.second == i) {
						func_name = p.first;
						break;
					}
				};
				return func_name;
			};
			const auto findClassIdentifier = [&](std::size_t i) {
				auto class_name = ClassIdentifier{ u"error",u"not found" };
				for (const auto &p : class_info_.link) {
					if (p.second == i) {
						class_name = p.first;
						break;
					}
				};
				return class_name;
			};
			Stream str;
			for (const auto &func_instance : call_stack_.get()) {
				const auto info = func_info_[func_instance.info_index];
				const auto func_name = findFuncIdentifier(func_instance.info_index);
				str << u"Function name = " << func_name.name << u"\n";
				str << u"Scope = " << func_name.scope << u"\n";
				str << u"Args = " << func_name.args.toString<Str>([](std::size_t s) {return toU16String(s); }) << u"\n";
				str << u"Command : \n";
				for (const auto &com : info.command) {
					str << Command::table[com.id] << u"(" << std::hex << std::uppercase << com.data.i32[0] << u","<< com.data.i32[1] << u")\n";
				}
				str << u"Memory : \n";
				for (const auto &param_pos : func_instance.memory_use) {
					str << u"Class is " << findClassIdentifier(param_pos.info_index).get() << u",\n";
					for (std::size_t i = 0; i < param_pos.length; i++) {
						str << std::hex << std::uppercase << func_instance.memory[param_pos.begin_of_memory + i];
						if (i % 32 == 31) {
							str << u"\n";
						}
					}
				}
				str << std::endl;
			}
			return str.str();
		}
		Command::Id executeCommand(Command *com) {
			decltype(auto) current_func = call_stack_.top();
			switch (com->id){
			case Command::ALLOC_CLASS:
				current_func.memory_use[com->data.i32[0]] = 
					allocate(current_func.memory, com->data.i32[1]);
				break;
			case Command::SUBSTITUTE:
				*static_cast<std::uint32_t*>(static_cast<void*>(current_func.memory.data() + current_func.memory_use[com->data.i32[0]].begin_of_memory)) = com->data.i32[1];
			default:
				break;
			}
			return com->id;
		}
		ParamPos allocate(Vector<std::uint8_t> &memory,std::size_t classinfo_index) {
			const auto old_size = memory.size();
			const auto class_size = class_info_[classinfo_index].size;
			memory.addSize(class_size);
			return ParamPos{ classinfo_index,old_size,class_size };
		}

	};
}


#endif //  CYM_HPP
