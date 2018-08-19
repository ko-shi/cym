#ifndef CYM_AST_HPP
#define CYM_AST_HPP

#include<cym/CymBase.hpp>
#include<cym/utils/CymTCPair.hpp>
#include<cym/lang/CymTrait.hpp>
#include<cym/utils/CymForwardIndexMap.hpp>
#include<cym/utils/string/CymStringConverter.hpp>

#include<map>

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


	enum class SentenceId {
		DEFINE_VAR,
		CALL_FUNC,
		RETURN
	};
	struct SentenceBase {
		virtual SentenceId id()const = 0;
		virtual Str toStr()const = 0;
		virtual ~SentenceBase() {
		}
	};
	struct SentenceDefineVariable : SentenceBase{

		Str name;
		Str trait;
		Size index;
		std::unique_ptr<ASTBase> initializer;
		SentenceDefineVariable(StrView n, StrView r, Size i, std::unique_ptr<ASTBase> &&init) : name(n), trait(r), index(i), initializer(std::move(init)) {

		}
		virtual Str toStr()const override {
			return Str(u"Define Variable{\n")
				+ Str(u"name = \"") + name + Str(u"\"\n")
				+ Str(u"trait = ") + trait + Str(u"\n")
				+ Str(u"index = ") + toU16String(index) + Str(u"\n")
				+ Str(u"initializer = ") + initializer->toStr() + Str(u"\n")
				+ Str(u"}\n");
		}
		virtual SentenceId id()const override {
			return SentenceId::DEFINE_VAR;
		}
	};
	struct SentenceNormal : SentenceBase {
		std::unique_ptr<ASTBase> ast;
		virtual Str toStr()const override {
			return ast->toStr();
		}
		virtual SentenceId id()const override {
			return SentenceId::DEFINE_VAR;
		}
	};



	struct ClassDef {
		bool is_functor;
		Str name;
		std::multimap<Str, ClassDef> member;

		FIndexMap<StrView> param_id;
		Vector<Pair<StrView, Trait>> args;
		Trait ret_trait;
		Vector<std::unique_ptr<SentenceBase>> order;

		ClassDef() = default;
		ClassDef(const ClassDef &) = default;
		ClassDef(ClassDef &&) = default;

		ClassDef(const Str &name) : is_functor(false), name(name) {

		}
		ClassDef(const Str &name,const Vector<Pair<StrView,Trait>> &args,const Trait &ret) : is_functor(true), name(name), args(args), ret_trait(ret) {

		}
		Str toStr()const {
			if (!is_functor) {
				return Str(u"field : name = ") + name;
			}
			Str o;
			for (const auto &i : order) {
				o += i->toStr();
			}
			Str m;
			for (const auto &i : member) {
				m += Str(u"{\n name = ") + i.first + u"\n" + i.second.toStr() + u"}\n";
			}
			return 
				Str(u"{\n functor : name = ") + name
				+ u"\norder : {" + o
				+ u"\n}\nmember : {" + m + u"}"
				+ u"\n}\n";
		}
	};

	enum ScopeKind {
		FUNC_SCOPE,
		CLASS_DEF_SCOPE,
		IF_SCOPE
	};
}

#endif
