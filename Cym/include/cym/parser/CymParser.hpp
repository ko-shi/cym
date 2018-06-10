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
	private:
	public:
		const StrView single_indent_ = u"    ";
		Vector<Scope> scope_;
		Vector<Str> code_;
		Size line_num_;
		StrView line_;
		Vector<ErrorMessage> error_;
		FuncDef ast_;
	public:
		Parser() : ast_{ u"main"} {

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
		const Scope& lastScope() const {
			return scope_.back().scope;
		}
		Scope& lastScope() {
			return scope_.back();
		}
		void scanScopeDef(Size line) {
			const Size depth_base = scopeDepth(line);
			for (Size i = line; i < code_.size(); i++) {
				const Size depth = scopeDepth(i);
				const StrView str = code_[0];
				if (depth_base == depth) {
					const auto token = takeWhile(removeSpace(str), [](Char c) {return c != u' '; });
					if (token == u"cls") {
						const auto name = takeNextToken(str, token);
						scope_.back().restrictions.emplaceBack(name);
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
			const auto &rests = scope_.back().restrictions;
			const auto head = takeToken(line_, infixes());
			if (std::find(rests.begin(), rests.end(), head) != rests.end()) {
				// Restriction
				const auto equal = takeNextToken(line_,takeNextToken(line_, head));
				if (equal == u"=") {
					// define variable
					caseDefineVar(line_, head);
				}
				else {
					if (head == u"func") {
						// define function template
						caseDefineFunc(line_, head);
					}
					else if (head == u"cls") {
						// define class template
					}
				}
			}
		}
		Size priority(StrView infix) const{
			const Map<StrView, Size> table =
			{ {u'+',10},{u'-',10},{u'*',20},{u'/',20} };
			const auto temp = table.find(infix);
			return temp == table.end() ? -1 : *temp;
		}
		void caseDefineVar(StrView str,StrView head) {
			if (lastScope().index() != FUNC_SCOPE) {
				error_.emplace_back(ErrorMessage::DEFINED_VARIABLE_IN_CLASS_SCOPE, line_num_, distance(str, head), str.substr(head.size()));
				return;
			}
			auto &func = *std::get<FUNC_SCOPE>(lastScope());
			const auto name = takeNextToken(str, head);
			const auto equal = takeNextToken(str, name);
			const auto initializer = getRemainedStr(str,equal);

			func.param_id.emplace(name);
			ASTDefVar ast_defvar(name, head, func.param_id[name]);
			ast_defvar.initializer = parseExpr(initializer, {});
			func.order.emplace_back(new ASTDefVar(std::move(ast_defvar)));
		}
		void caseDefineFunc(StrView str,StrView head) {
			if (lastScope().index() != FUNC_SCOPE) {
				error_.emplace_back(ErrorMessage::DEFINED_FUNCTION_IN_CLASS_SCOPE, line_num_, distance(str, head), str.substr(head.size()));
				return;
			}
			auto &func = *std::get<FUNC_SCOPE>(lastScope());
			const auto declaration = takeNextToken(str, head);
			const auto name = toFuncName(declaration);
			const auto args = listArgs(declaration);


			FuncDef new_func{};
			new_func.name = Str(name);
			for (const auto &arg : args) {
				new_func.args.emplace_back()
			}

			func.inner_func
		}
		std::unique_ptr<ASTBase> parseExpr(StrView expr,Vector<std::unique_ptr<ASTBase>> &&former,Size prev_priority = -1) {
			const auto recurse = [&](StrView hind) {
			};

			if (lastScope().index() != FUNC_SCOPE) {
				error_.emplace_back(ErrorMessage::WRITTEN_EXPRESSION_IN_CLASS_SCOPE, line_num_, distance(line_, expr), expr);
				return std::make_unique<ASTBase>();
			}
			const auto token = takeToken(expr);
			const auto kind = getTokenKind(token);
			auto hind = getRemainedStr(expr, token);

			if (former.size() % 2 == 0) {
				// token must not be infix
				switch (kind) {
				case TokenKind::NUMBER:
					former.emplace_back(new ASTNum(toInt(token)));
					break;
				case TokenKind::VARIABLE: {
					auto &func = *std::get<FUNC_SCOPE>(lastScope());

					if (func.param_id.exist(token)) {
						former.emplace_back(new ASTVar(Str(token), func.param_id[token]));
					}
					else {
						// 未実装 : 外部変数の取得
						// この関数はunreferableになる
						error_.emplace_back(ErrorMessage::UNKNOWN_VARIABLE, line_num_, distance(line_, token), token);
					}
					break;
				}
				case TokenKind::FUNC: {
					ASTCallFunc call_func;
					call_func.name = toFuncName(token);
					for (const auto arg : listArgs(token)) {
						call_func.args.emplace_back(parseExpr(arg, {}));
					}
					former.emplace_back(new ASTCallFunc(std::move(call_func)));
					break;
				}
				case TokenKind::EXPRESSION:
					former.emplace_back(parseExpr(bracketContent(token), {}));
					break;
				}
			}
			else {
				// token must be infix
				if (kind != TokenKind::VARIABLE) {
					error_.emplace_back(ErrorMessage::CONCECTIVE_NONINFIXES, line_num_m, distance(line_, token), token);
				}
				else {
					const auto now_priority = priority(token);
					if (now_priority <= prev_priority) {
						former.emplace_back(new ASTUnsolvedInfix(token));
					}
					else {
						const auto next = takeNextToken(expr, token);
						if (next.empty()) {
							error_.emplace_back(ErrorMessage::ENDED_WITH_INFIX, line_num_, distance(line_, token), token);
							return solve();
						}
						former.back().reset(new ASTInfix(token, std::move(former.back()), parseExpr(next));
						hind = takeNextToken(expr, next);
					}
					prev_priority = now_priority;
				}
			}
			hind = removeSpace(hind);
			if (hind.empty()) {
				if (former.size() % 2 == 1) {
					error_.emplace_back(ErrorMessage::ENDED_WITH_INFIX, line_num_, distance(line_, token), token);
					former.pop_back();
				}
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
		}

	};

}

#endif
