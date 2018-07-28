#ifndef CYM_CODE_GENERATOR
#define CYM_CODE_GENERATOR

#include<cym/lang/CymAST.hpp>
#include<cym/vm/CymOpcode.hpp>
#include<cym/lang/CymInstancedType.hpp>
#include<cym/utils/CymForwardIndexMap.hpp>

namespace cym {
	class CodeGen {
		ByteCode byte_code_;
		FuncDef ast_;
		CodeGen(FuncDef &&a) : ast_(std::move(a)) {

		}
		FuncDef* findFunc(const Vector<Scope> &s,StrView name) {
			for (auto i = s.rbegin(); i != s.rend(); i++) {
				if (i->index() == ScopeKind::FUNC_SCOPE) {
					const auto &func = std::get<ScopeKind::FUNC_SCOPE>(*i);
					func->inner_func.
				}
			}
		}
		void generate() {
			Vector<Scope> stratum;
			stratum.emplace_back(&ast_);
			for (const auto &i : ast_.order) {
				switch (i->id()) {
				case ASTId::CALL_FUNC: {
					const auto ast = dynamic_cast<ASTCallFunc*>(i.get());
					ast->name;
				}
				}
			}
		}
	};
}

#endif