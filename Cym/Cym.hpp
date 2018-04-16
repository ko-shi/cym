#ifndef CYM_HPP
#define CYM_HPP

#include<string>
#include<vector>
#include<algorithm>
#include<functional>

#include"CymVector.hpp"
#include"CymStack.hpp"
#include"CymDoubleKeyMap.hpp"
#include"CymDataTypes.hpp"
#include"CymStringConverter.hpp"
#include"CymHandStack.hpp"
#include"CymStringOperation.hpp"

namespace cym {

	struct SSPairHash {
		std::size_t operator()(const std::pair<std::size_t, std::size_t> &p) const {
			return std::hash<std::size_t>()(p.first) ^ std::hash<std::size_t>()(p.second);
		}
	};

	class Cym {
	public:

	//private:
	public:
		// compile time
		std::vector<Str> code_;
		Vector<WordInfo> word_info_;
		DoubleKeyMap<std::pair<std::size_t, std::size_t>/* line and pos */, ParamIdentifier,SSPairHash> param_identifier_;
		std::unordered_map<ForSameName, std::size_t/* times */> param_name_times_;
		std::unordered_map<ParamIdentifier, Vector<Pair<TokenKind, Str>>> to_infer_list_;
		Vector<Str> intermediate_code_;

		DoubleKeyMap<ClassIdentifier, ClassInfo> class_info_;
		DoubleKeyMap<FuncIdentifier, FuncInfo> func_info_;
		std::unordered_map<StrView, std::size_t> priorities_;
		Vector<Str> operators_;
		Vector<Str> reserved_words_;

		// runtime
		std::size_t current_scope;
		Stack<FuncInstance> call_stack_;
		HandStack</* class_info_'s index */std::size_t> hand_stack_;
	public:
		Cym() {
			const std::pair<StrView, std::size_t> default_operators[]
				= { { u"(",2 },{ u")",2 },{ u"=",16 },{ u"+",6 },{ u"-",6 },{ u"*",5 },{ u"/",5 } };
			for (const auto &i : default_operators) {
				priorities_.emplace(i);
			}
			operators_ = Vector<Str>{ u"(", u")", u"=", u"+", u"-", u"*", u"/" };
			reserved_words_ = Vector<Str>{ u"var" };
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
		Vector<Pair<TokenKind,Str>> getRPN(const StrView &str) {
			const auto compPriority = [&](const StrView &l, const StrView &r) {return priorities_[l] > priorities_[r]; };
			return Vector<Pair<TokenKind, Str>>{};
		}
		ClassIdentifier requestToInferType(const ParamIdentifier &param) {
			for (auto &&i : to_infer_list_[param]) {
				switch (i.first) {
				case TokenKind::NUMBER:
					i = makePair(TokenKind::TYPEDETERMINED, ClassIdentifier{ u"master",u"PrimaryInt" }.get());
					break;
					// TODO 文字列リテラルの推論、関数、中置関数の推論
				}
			}
			const auto last_pair = to_infer_list_[param].back();
			if (to_infer_list_[param].size() != 1 || last_pair.first != TokenKind::TYPEDETERMINED) {
				return ClassIdentifier{ u"error",u"type couldn't be infered." };
			}
			return makeCIFromSingleStr(last_pair.second);
		}

		int compileLine(const Str &str,std::size_t line) {
			using namespace std::placeholders;
			TokenKind kind;
			auto pickUpWord = [](StrView) {return StrView{}; };
			auto nextWord = [](StrView,TokenKind&) {return StrView{}; };
			
			auto scope = Str(u"master/Main/main");
			const auto head = pickUpWord(str);
			switch (kind) {
			case TokenKind::RESERVEDWORD: {
				WordInfo reserved_word_word_info;
				reserved_word_word_info.kind = kind;
				reserved_word_word_info.name = head;
				reserved_word_word_info.place = std::make_pair(line,str.find(head));
				word_info_.emplaceBack(reserved_word_word_info);
				if (head == u"var") {
					/* Definition of a paramator */
					const auto var_name = StrView{};//seekToNextWord(str, head, reserved_words_, operators_, kind);
					if (kind != TokenKind::PARAM) {
						//TODO コンパイルエラー処理
					}
					const auto name_place = std::make_pair(line, str.find(head));
					
					const auto var_identifier = ParamIdentifier{ scope,Str(var_name), ++param_name_times_[ForSameName{ scope,Str(var_name) }]};
					param_identifier_.emplace(name_place, var_identifier);
					WordInfo var_name_word_info{kind,Str(var_name),name_place,param_identifier_.indexOf(name_place)};
					const auto equal = StrView{};//seekToNextWord(StrView(str), StrView(var_name), reserved_words_, operators_, kind);

					if (equal != u"=") {
						//TODO コンパイルエラー処理
					}
					const auto rpn = getRPN(nextWord(equal, kind));
					to_infer_list_.emplace(var_identifier, rpn);

					intermediate_code_.emplaceBack(Str(u"allocate") + u" " + var_identifier.get() + u" " + u"not_determined");
					intermediate_code_.emplaceBack(Str(u"push") + u" " + u"param" + u" " + var_identifier.get());
					for (const auto &i : rpn) {
						intermediate_code_.emplaceBack(
							Str(u"push") + u" " + TokenClass_table[static_cast<std::size_t>(i.first)] + u" " + i.second
						);
					}
					intermediate_code_.emplaceBack(Str(u"call") + u" " + u"typeof" + u"(" + var_identifier.get() + u")" + u"." + u"constructor@");
				}
			}
			}
			return 0;
		}
		int compile() {
			addClass(u"master", u"PrimaryInt",/* size = */ 4,/* param_num = */ 1,/* params */ {},/* funcs */ {});
			const auto primary_int_index = class_info_.indexOf({ u"master",u"PrimaryInt" });
			addFunc(u"master/PrimaryInt", { primary_int_index/* TODO :reference ,means 'this' */,primary_int_index }, u"@constructor@", 1, 4, {
				Command(Command::PICK_UP, 0,0)
			});
			std::size_t line = 0;
			for (const auto &i : code_) {
				compileLine(i,line);
				line++;
			}
			addClass(u"master", u"Main", 0, 0, {}, {0});
			addFunc(u"master/Main", {}, u"main", 1, 4, {
				Command(Command::ALLOC_CLASS, 0, class_info_.indexOf({u"master",u"PrimaryInt"})),
				Command(Command::SUBSTITUTE, 0, 0x01234567)
			});

			return 0;
		}
		int run() {
			const auto main_func_identifier = FuncIdentifier{ Str(u"master/Main"),Vector<std::size_t>{},Str(u"main") };
			auto main_func_info = func_info_[main_func_identifier];
			call_stack_.emplace(func_info_.indexOf(main_func_identifier), main_func_info.param_num, main_func_info.default_size);
			
			for (auto com = main_func_info.command.begin(); com != main_func_info.command.end(); com++) {
				executeCommand(com);
			}
			return 0;
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
				break;
			case Command::PICK_UP:// TODO: int32のみ代入演算子でコピーする
				std::memcpy(
					current_func.memory.data() + current_func.memory_use[com->data.i32[0]].begin_of_memory,
					hand_stack_.backAsArrayBegin(),
					hand_stack_.getLastSize() // This must be the same as class_info_[current_func.memory_use[com->data.i32[0]].info_index].size
				);
				hand_stack_.popBack();
				break;
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


		/* FUNCTIONS FOR DEBUG */
		Str showIntermediateCode()const {
			Str str;
			for (const auto &i : intermediate_code_) {
				str += i + u"\n";
			}
			return str;
		}
		Str showInferedTypes() {
			Str str;
			for (const auto &i : to_infer_list_) {
				str += i.first.name + u" = ";
				str += i.second.toString<Str>([](const cym::Pair<cym::TokenKind, cym::Str> &p) {
					return Str(u"[") + Str(cym::TokenClass_table[static_cast<std::size_t>(p.first)]) + Str(u",") + p.second + Str(u"]");
				});
			}
			return str;
		}
		Str showMemory() {
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
					str << Command::table[com.id] << u"(" << std::hex << std::uppercase << com.data.i32[0] << u"," << com.data.i32[1] << u")\n";
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
	};
}


#endif //  CYM_HPP
