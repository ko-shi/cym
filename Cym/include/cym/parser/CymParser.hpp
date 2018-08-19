#ifndef CYM_PARSER_HPP
#define CYM_PARSER_HPP


#include<cym/CymBase.hpp>
#include<cym/utils/CymTCVector.hpp>
#include<cym/lang/CymAST.hpp>
#include<cym/parser/CymLexer.hpp>
#include<cym/utils/CymNumConverter.hpp>
#include<cym/lang/CymErrorMessage.hpp>

namespace cym {

	class Parser {
	public:
		using Itr = Vector<Str>::iterator;
	private:
	public:
		const StrView single_indent_ = u"    ";
		Vector<Str> code_;
		Size line_num_;
		StrView line_;
		Vector<ErrorMessage> error_;
		ClassDef main_class_;
	public:
		Parser() : main_class_(u"main_class", {}, Trait{}) {
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
		Size countDepth(StrView line) {
			return countStr(line, single_indent_);
		}
		Pair<Itr, Itr> cutOffScope(Itr begin,Itr end,Size depth) {
			for (auto itr = begin; itr != end; itr++) {
				Size d = countDepth(*itr);
				if (!getRemainedStr(*itr,StrView(*itr).substr(d * single_indent_.length())).empty()
					&& d >= depth) {
					return makePair(begin, itr);
				}
			}
			return makePair(begin, end);
		}
		Trait parseTrait(StrView) {
			return Trait();
		}
		void addError(ErrorMessage::Kind kind, StrView point, const Str &m = Str(u"")) {
			error_.emplace_back(kind, line_num_, distance(line_, point), point);
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

			Vector<ClassDef*> stratum{&main_class_};
			parseFuncDefinition(u"func ()", makePair(code_.begin(), code_.end()),stratum);
		}

		Itr parseLine(Itr line, Itr scope_end, Size depth, const Vector<ClassDef*> &stratum) {
			const auto l = StrView(*line);
			auto first = takeToken(l);
			auto second = takeNextToken(l, first);
			auto third = takeNextToken(l, second);
			auto fourth = takeNextToken(l, third);
			if (second == u":" && fourth == u"=") {
				parseVariableDefinition(l, first,third,getRemainedStr(l,fourth),stratum);
				return line;
			}
			else if (second == u"=") {
				parseVariableDefinition(l, first, StrView(), getRemainedStr(l, second),stratum);
				return line;
			}
			else if (first == u"func" && getTokenKind(second) == TokenKind::FUNC) {
				const auto range = cutOffScope(line + 1, scope_end, depth);
				return parseFuncDefinition(l,range,stratum);
			}
			throw "not concerned sentence";
			return Itr();
		}
		Size priority(StrView infix) const{
			const Map<StrView, Size> table =
			{ {u"+",10},{u"-",10},{u"*",20},{u"/",20} };
			const auto temp = table.find(infix);
			return temp == table.end() ? -1 : temp->second;
		}
		void parseVariableDefinition(StrView str,StrView name,StrView trait,StrView initializer, const Vector<ClassDef*> &stratum) {
			auto &scope = *stratum.back();
			Size id = scope.param_id.emplace(str);
			scope.order.emplace_back(std::make_unique<SentenceDefineVariable>(name, trait, id, parseExpr(initializer, stratum, {})));

		}
		Itr parseFuncDefinition(StrView declaration, Pair<Itr,Itr> range, const Vector<ClassDef*> &stratum) {
			auto first = takeToken(declaration);
			auto second = takeNextToken(declaration, first);

			const auto name = toFuncName(second);
			const auto args = listArgs(declaration);

			Vector<Pair<StrView, Trait>> separated(args.size());
			for (auto arg : args) {
				const auto arg_name = takeToken(arg);
				const auto colon = takeNextToken(arg, arg_name);
				const auto trait = colon.empty() ? StrView() : takeNextToken(arg, colon);
				separated.emplace_back(arg_name, Trait(Str(trait)));
			}

			auto def_itr = stratum.back()->member.emplace(name, ClassDef(name, separated, Trait()));
			auto new_stratum = stratum;
			new_stratum.push_back(&def_itr->second);
			for (auto itr = range.first; itr != range.second; itr++) {
				itr = parseLine(itr, range.second, 0, new_stratum);
			}
			return range.second;
		}
		std::unique_ptr<ASTBase> parseExpr(StrView expr,const Vector<ClassDef*> &stratum, Vector<std::unique_ptr<ASTBase>> &&former,Size prev_priority = -1) {

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
					auto &func = *stratum.back();

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
						call_func.args.emplace_back(parseExpr(arg, stratum, {}));
					}
					former.emplace_back(new ASTCallFunc(std::move(call_func)));
					break;
				}
				case TokenKind::EXPRESSION:
					former.emplace_back(parseExpr(bracketContent(token), stratum, {}));
					break;
				}
			}
			else {
				// token must be infix
				if (kind != TokenKind::VARIABLE) {
					error_.emplace_back(ErrorMessage::CONCECTIVE_NONINFIXES, line_num_, distance(line_, token), token);
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
						}
						else {
							former.back().reset(new ASTInfix(token, std::move(former.back()), parseExpr(next, stratum, {})));
							hind = takeNextToken(expr, next);
						}
					}
					prev_priority = now_priority;
				}
			}
			hind = removeSpace(hind);
			if (hind.empty()) {
				if (former.size() % 2 == 0 && !former.empty()) {
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
			return parseExpr(hind, stratum, std::move(former), prev_priority);
		}

	};

}

#endif
