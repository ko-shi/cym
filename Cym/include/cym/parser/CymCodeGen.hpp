#ifndef CYM_CODE_GENERATOR
#define CYM_CODE_GENERATOR

#include<cym/lang/CymAST.hpp>
#include<cym/vm/CymOpcode.hpp>
#include<cym/lang/CymInstancedType.hpp>
#include<cym/utils/CymForwardIndexMap.hpp>
#include<cym/utils/CymDoubleKeyMap.hpp>

namespace cym {
	class CodeGen {
		ByteCode byte_code_;
		FuncDef ast_;
		DoubleKeyMap<FuncIdentifier,ByteCodeFunc> defined_operators_;
		CodeGen(FuncDef &&a) : ast_(std::move(a)) {
			defined_operators_.emplace(FuncIdentifier(u"+", InstancedType{ FunctionType{} });
		}
		FuncDef* findFunc(const Vector<Scope> &s,const Str &name) {
			// TODO: オーバーロード対応
			for (auto i = s.rbegin(); i != s.rend(); i++) {
				if (i->index() == ScopeKind::FUNC_SCOPE) {
					const auto &func = std::get<ScopeKind::FUNC_SCOPE>(*i);
					if (func->inner_func.count(name)) {
						return &func->inner_func[name];
					}
				}
			}
		}
		void generate() {
			Vector<Scope> stratum;
			stratum.emplace_back(&ast_);
			ByteCodeFunc func_code;
			for (const auto &i : ast_.order) {
				switch (i->id()) {
				case ASTId::CALL_FUNC: {
					const auto ast = dynamic_cast<ASTCallFunc*>(i.get());
					findFunc(stratum,ast->name);
				}
				}
			}
		}
		void generateCallFunc(/* Copy constructing */Vector<Scope> stratum,const ASTCallFunc &func,ByteCodeFunc &caller) {
			caller.com.push_back
		}
	};
}

#endif