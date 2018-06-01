#ifndef CYM_AST_HPP
#define CYM_AST_HPP

#include<cym/CymBase.hpp>
#include<cym/utils/CymVector.hpp>
#include<cym/lang/CymRestriction.hpp>
#include<cym/utils/CymTCPair.hpp>
#include<cym/utils/CymForwardIndexMap.hpp>

#include<variant>

namespace cym {
	enum class ASTId {
		BASE,
		ERROR,
		NUM,
		DEF_PARAM,
		CALL_FUNC,
	};

	struct ASTBase {
		virtual ASTId id() const{
			return ASTId::BASE;
		}
		virtual ~ASTBase() {

		}
	};
	struct ASTCompileError : ASTBase {
		Str message;
		virtual ASTId id() const {
			return ASTId::ERROR;
		}
	};
	struct ASTNum : ASTBase {
		Int i;

		virtual ASTId id() const override {
			return ASTId::NUM;
		}
	};
	struct ASTDefParam : ASTBase {
		Str name;
		Size index;
		std::unique_ptr<ASTBase> initializer;
		ASTDefParam(const Str &n, Size i) : name(n), index(i) {

		}
		virtual ASTId id()const override {
			return ASTId::DEF_PARAM;
		}
	};
	struct ASTCallFunc : ASTBase {
		Str name;
		Vector<Str> args;
		virtual ASTId id()const override {
			return ASTId::CALL_FUNC;
		}
	};

	struct FuncDef;
	struct ClassDef;

	struct FuncDef {
		Str name;
		FIndexMap<StrView> param_id;

		Vector<Pair<Str, Restriction>> args;
		Restriction ret_rest;
		Vector<std::unique_ptr<ASTBase>> order;

		Vector<std::unique_ptr<FuncDef>> inner_func;
		Vector<std::unique_ptr<ClassDef>> inner_cls;
	};
	struct ClassDef {
		Size cls_id;
		Str name;
		Vector<Pair<Str, Restriction>> mem_params;
		Vector<FuncDef> mem_funcs;

		Vector<std::unique_ptr<ClassDef>> inner_cls;
	};

	enum ScopeKind {
		FUNC_SCOPE,
		CLASS_SCOPE
	};
	using Scope = std::variant<
		FuncDef*,
		ClassDef*
	>;
	
}

#endif
