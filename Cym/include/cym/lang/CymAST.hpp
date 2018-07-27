#ifndef CYM_AST_HPP
#define CYM_AST_HPP

#include<cym/CymBase.hpp>
#include<cym/utils/CymTCPair.hpp>
#include<cym/lang/CymTrait.hpp>
#include<cym/utils/CymForwardIndexMap.hpp>
#include<cym/utils/string/CymStringConverter.hpp>

#include<variant>

#ifdef ERROR
#	undef ERROR
#endif

namespace cym {
	enum class ASTId {
		BASE,
		ERROR,
		NUM,
		DEF_VAR,
		VAR,
		EXTERNAL_VAR,
		CALL_FUNC,
		UNSOLVED_INFIX,
		INFIX,
	};

	struct ASTBase {
		virtual ASTId id() const{
			return ASTId::BASE;
		}
		virtual Str toStr()const {
			return u"(base)\n";
		}
		virtual ~ASTBase() {

		}
	};
	struct ASTNum : ASTBase {
		Int i;
		ASTNum(Int _i) : i(_i){
		
		}
		virtual Str toStr()const override {
			return Str(u"Number : ") + toU16String(i);
		}
		virtual ASTId id() const override {
			return ASTId::NUM;
		}
	};
	struct ASTDefVar : ASTBase {
		Str name;
		Str trait;
		Size index;
		std::unique_ptr<ASTBase> initializer;
		ASTDefVar(StrView n,StrView r,Size i) : name(n),trait(r), index(i) {

		}
		virtual Str toStr()const override {
			return Str(u"Define Variable{\n")
				+ Str(u"name = \"") + name + Str(u"\"\n")
				+ Str(u"trait = ") + trait + Str(u"\n")
				+ Str(u"index = ") + toU16String(index) + Str(u"\n")
				+ Str(u"initializer = ") + initializer->toStr() + Str(u"\n")
				+ Str(u"}\n");
		}
		virtual ASTId id()const override {
			return ASTId::DEF_VAR;
		}
	};
	struct ASTVar : ASTBase {
		Str name;
		Size index;
		ASTVar(const Str &n, Size i) : name(n), index(i) {

		}
		virtual Str toStr()const override {
			return Str(u"Variable{")
				+ Str(u"name = \"") + name + Str(u"\",")
				+ Str(u"index = ") + toU16String(index) + Str(u"}\n");
		}
		virtual ASTId id()const override {
			return ASTId::VAR;
		}
	};
	struct ASTExternalVar : ASTBase {
		Str name;
		Size retrace_num;
		Size index;
		ASTExternalVar(const Str &n, Size r, Size i) : name(n), retrace_num(r), index(i) {

		}
		virtual Str toStr()const override {
			return Str(u"External Variable{\n")
				+ Str(u"name = \"") + name + Str(u"\"\n")
				+ Str(u"index = ") + toU16String(index) + Str(u"\n")
				+ Str(u"}\n");
		}
		virtual ASTId id()const override {
			return ASTId::EXTERNAL_VAR;
		}
	};
	struct ASTCallFunc : ASTBase {
		Str name;
		Vector<std::unique_ptr<ASTBase>> args;
		virtual Str toStr()const override {
			Str temp;
			for (auto &i : args) {
				temp += i->toStr() + u",";
			}
			if (!temp.empty())temp.pop_back();
			return Str(u"Call Function{\n")
				+ Str(u"name = \"") + name + Str(u"\"\n")
				+ Str(u"args = ") + temp + Str(u"\n")
				+ Str(u"}");
		}
		virtual ASTId id()const override {
			return ASTId::CALL_FUNC;
		}
	};
	struct ASTUnsolvedInfix : ASTBase {
		StrView name;
		ASTUnsolvedInfix(StrView s) : name(s) {

		}
		virtual Str toStr()const override {
			return Str(u"Unsolved Infix{\n")
				+ Str(u"name = \"") + Str(name) + Str(u"\"\n")
				+ Str(u"}\n");
		}
		virtual ASTId id()const override {
			return ASTId::UNSOLVED_INFIX;
		}
	};
	struct ASTInfix : ASTBase {
		StrView name;
		std::unique_ptr<ASTBase> l, r;

		ASTInfix(StrView s) : name(s), l(), r() {

		}
		ASTInfix(StrView s, std::unique_ptr<ASTBase> &&_l, std::unique_ptr<ASTBase> &&_r) : name(s), l(std::move(_l)), r(std::move(_r)) {

		}
		virtual Str toStr()const override {
			return Str(u"Call Infix{\n")
				+ Str(u"name = \"") + Str(name) + Str(u"\"\n")
				+ Str(u"left = ") + (l ? l->toStr() : Str(u"Error:null")) + Str(u"\n")
				+ Str(u"right = ") + (r ? r->toStr() : Str(u"Error:null")) + Str(u"\n")
				+ Str(u"}");
		}
		virtual ASTId id()const override {
			return ASTId::INFIX;
		}
	};

	struct FuncDef;
	struct ClassDef;

	struct FuncDef {
		Str name;
		FIndexMap<StrView> param_id;
		// bool unreferable = false;
		Vector<Pair<StrView, Trait>> args;
		Trait ret_rest;
		Vector<std::unique_ptr<ASTBase>> order;

		Vector<FuncDef> inner_func;
		Vector<ClassDef> inner_cls;

		Str toStr()const{
			Str temp;
			for (const auto &i : order) {
				temp += i->toStr();
			}
			return Str(u"name = ") + name + Str(u"\n")
				+ Str(u"ast = ") + temp;
		}
	};
	struct ClassDef {
		Size cls_id;
		Str name;
		Vector<Pair<Str, Trait>> mem_params;
		Vector<FuncDef> mem_funcs;

		Vector<std::unique_ptr<ClassDef>> inner_cls;
	};

	enum ScopeKind {
		FUNC_SCOPE,
		CLASS_SCOPE
	};
	using Scope = Variant<
		FuncDef*,
		ClassDef*
	>;
	
}

#endif
