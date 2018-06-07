#ifndef CYM_PARSER_HPP
#define CYM_PARSER_HPP


#include<cym/CymBase.hpp>
#include<cym/utils/CymVector.hpp>
#include<cym/lang/CymAST.hpp>
#include<cym/parser/CymLexer.hpp>
#include<cym/utils/CymNumConverter.hpp>
#include<cym/lang/CymErrorMessage.hpp>

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
	public:
		struct ScopeSpecDef {
			Map<StrView,Size> infixes;//Size means priority
			Vector<StrView> restrictions;
		};

		const StrView single_indent_ = u"    ";
		Vector<ScopeSpecDef> scope_def_;
		Vector<Str> code_;
		Size line_num_;
		StrView line_;
		Vector<ErrorMessage> error_;
		FuncDef ast_;
		Scope cur_scope_;
	public:
		Parser() : ast_{ u"main"} {
			scope_def_.push_back({ {{u"+",10},{u"-",10},{u"*",20},{u"/",20}},{u"func",u"var"} });
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
						// エラー : 最初の行で'/'
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
						scope_def_.back().restrictions.emplaceBack(name);
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

			line_num_ = 0;
			for (const auto &s : code_) {
				line_ = StrView(s);
				parseLine();
				line_num_++;
			}
		}
		void parseLine() {
			const auto &rests = scope_def_.back().restrictions;
			const auto head = takeToken(line_, infixes());
			if (std::find(rests.begin(), rests.end(), head) != rests.end()) {
				// Restriction
				const auto equal = takeNextToken(line_,takeNextToken(line_, head, infixes()),infixes());
				if (equal == u"=") {
					// define param
					caseDefineParam(line_, head);
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
		const Map<StrView, Size>& infixes()const {
			return scope_def_.back().infixes;
		}
		const Vector<StrView>& restrictions()const {
			return scope_def_.back().restrictions;
		}
		void caseDefineParam(StrView str,StrView head) {
			if (cur_scope_.index() != FUNC_SCOPE) {
				return;
			}
			auto &func = *std::get<FUNC_SCOPE>(cur_scope_);
			const auto name = takeNextToken(str, head, infixes());
			const auto equal = takeNextToken(str, name, infixes());
			const auto initializer = getRemainedStr(str,equal);

			func.param_id.emplace(name);
			ASTDefVar ast_defvar(name, head, func.param_id[name]);
			ast_defvar.initializer = parseExpr(initializer, {});
			func.order.emplace_back(new ASTDefVar(std::move(ast_defvar)));
		}
		std::unique_ptr<ASTBase> parseExpr(StrView expr,Vector<std::unique_ptr<ASTBase>> &&former,Size prev_priority = -1) {
			const auto recurse = [&](StrView hind) {
				hind = removeSpace(hind);
				if (hind.empty()) {
					while (former.size() > 2) {
						auto &first = former[0];
						auto &second = former[1];
						auto &third = former[2];

						if (second->id() == ASTId::UNSOLVED_INFIX) {
							const auto infix = dynamic_cast<ASTUnsolvedInfix*>(second.get());
							third.reset(new ASTInfix(infix->name, std::move(first), std::move(third)));

							former.erase(former.begin());
							former.erase(former.begin());
						}
					}
					if (former.size() == 1) {
						
						return std::move(former[0]);
					}
					error_.emplace_back(ErrorMessage::ENDED_WITH_INFIX, line_num_, distance(line_, expr), expr);
					return std::make_unique<ASTBase>();
				}
				return parseExpr(hind, std::move(former), prev_priority);
			};

			if (cur_scope_.index() != FUNC_SCOPE) {
				return std::make_unique<ASTBase>();
			}
			auto &func = *std::get<FUNC_SCOPE>(cur_scope_);

			const auto token = takeToken(expr, infixes());
			const auto kind = getTokenKind(token, infixes());
			auto hind = getRemainedStr(expr, token);


			if (kind == TokenKind::INFIX) {
				if (former.empty()) {
					error_.emplace_back(ErrorMessage::BEGUN_WITH_INFIX,line_num_,distance(line_,token),token);
					return recurse(hind);
				}
				else if (former.back()->id() == ASTId::UNSOLVED_INFIX) {
					error_.emplace_back(ErrorMessage::CONCECTIVE_INFIXES, line_num_, distance(line_, token), token);
					return recurse(hind);
				}
			}
			else if (!former.empty() && former.back()->id() != ASTId::UNSOLVED_INFIX) {
				error_.emplace_back(ErrorMessage::CONCECTIVE_NONINFIXES, line_num_, distance(line_, token), token);
				return recurse(hind);
			}

			switch (kind) {
			case TokenKind::NUMBER:
				former.emplace_back(new ASTNum(toInt(token)));
						break;
			case TokenKind::VAR:
				if (func.param_id.exist(token)) {
					former.emplace_back(new ASTVar(Str(token), func.param_id[token]));
				}
				else {
					// 未実装 : 外部変数の取得
					// この関数はunreferableになる
					error_.emplace_back(ErrorMessage::UNKNOWN_VARIABLE, line_num_, distance(line_, token), token);
				}
				break;
			case TokenKind::FUNC: {
				ASTCallFunc call_func;
				call_func.name = toFuncName(token, infixes());
				for (const auto arg : listArgs(token, infixes())) {
					call_func.args.emplace_back(parseExpr(arg, {}));
				}
				former.emplace_back(new ASTCallFunc(std::move(call_func)));
				break;
			}
			case TokenKind::EXPRESSION:
				former.emplace_back(parseExpr(bracketContent(token), {}));
				break;
			case TokenKind::INFIX:
				const auto now_priority = scope_def_.back().infixes[token];
				if (prev_priority < now_priority) {
					const auto right_hand = takeToken(hind, infixes());
					former.back().reset(new ASTInfix(Str(token), std::move(former.back()), parseExpr(right_hand, {})));
					hind = getRemainedStr(expr, right_hand);
				}
				else {
					former.emplace_back(new ASTUnsolvedInfix(Str(token)));
				}
				prev_priority = now_priority;
				break;
			}
			return recurse(hind);
		}

	};

}

#endif
