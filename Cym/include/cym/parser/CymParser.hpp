#ifndef CYM_PARSER_HPP
#define CYM_PARSER_HPP

#include<cym/CymBase.hpp>
#include<cym/utils/CymVector.hpp>
#include<cym/lang/CymAST.hpp>
#include<cym/parser/CymLexer.hpp>

namespace cym {

	class Parser {
	public:
		enum Kind {
			FUNCTION,
			PARAM,
			INFIX,
			RESTRICTION,

		};
	private:
		struct ScopeSpecDef {
			Vector<StrView> infixes;
			Vector<StrView> restrictions;
		};

		const StrView single_indent_ = u"    ";
		Vector<ScopeSpecDef> scope_def;
		Vector<Str> code_;
		FuncDef ast_;
		Scope cur_scope_;
	public:
		Parser() : ast_{ u"main"} {
			scope_def.push_back({ {u"+",u"-",u"*",u"/"},{u"func",u"var"} });
			cur_scope_ = &ast_;
		}
		void addCode(const Str &code) {
			const auto head = removeSpace(code);
			if (!head.empty() && head[0] == u'/') {
				if (head.size() <= 1 || (head.size() > 1 && head[1] != u'/')) {
					if (!code_.empty()) {
						code_.back() += Str(head);
					}
					else {
						// ÉGÉâÅ[ : ç≈èâÇÃçsÇ≈'/'
					}
				}
			}
			code_.emplace_back(code);
		}
		Size scopeDepth(Size line) {
			return countStr(code_[line], single_indent_);
		}
		void scanScopeDef(Size line) {
			const Size depth_base = scopeDepth(line);
			for (Size i = line; i < code_.size(); i++) {
				const Size depth = scopeDepth(i);
				const StrView str = code_[0];
				if (depth_base == depth) {
					const auto token = takeWhile(removeSpace(str), [](Char c) {return c != u' '; });
					if (token == u"cls") {
						const auto name = takeNextToken(str, token, {});
						scope_def.back().restrictions.emplaceBack(name);
					}
				}
				if (depth_base > depth) {
					if (!getRemainedStr(str, takeWhile(str, [](Char c) {return c == u' '; })).empty()) {
						break;
					}
				}
			}
		}
		void parse() {
			for (auto &&s : code_) {
				for (Size i = 0; i < s.size(); i++) {
					if (s[i] == u'\t') {
						s.replace(i, 1, single_indent_);
						i += single_indent_.size() - 1;
					}
				}
			}

			scanScopeDef(0);

			Size index = 0;
			for (const auto &s : code_) {
				parseLine(StrView(s), index);
				index++;
			}
		}
		void parseLine(StrView str,Size line) {
			const auto &infixes = scope_def.back().infixes;
			const auto &rests = scope_def.back().restrictions;
			const auto head = takeToken(str, infixes);
			if (std::find(rests.begin(), rests.end(), head) != rests.end()) {
				// Restriction
				const auto equal = takeNextToken(str,takeNextToken(str, head, infixes),infixes);
				if (equal == u"=") {
					// define param
					caseDefineParam(str, head, line);
				}
				else {
					if (head == u"func") {
						// define function template
					}
					else if (head == u"cls") {
						// define class template
					}
				}
			}
		}
		void caseDefineParam(StrView str,StrView head,Size line) {
			if (cur_scope_.index() != FUNC_SCOPE) {
				return;
			}
			const auto &infixes = scope_def.back().infixes;
			auto &func = *std::get<FUNC_SCOPE>(cur_scope_);
			const auto name = takeNextToken(str, head, infixes);

			func.param_id.emplace(name);
			func.order.push_back(std::make_unique<ASTDefParam>(Str(name),func.param_id[name]));
		}
		std::unique_ptr<ASTBase> parseExpr(StrView expr) {

		}

	};

}

#endif
