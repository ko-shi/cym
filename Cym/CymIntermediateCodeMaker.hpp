#ifndef CYM_INTERMEDIATE_CODE_MAKER_HPP
#define CYM_INTERMEDIATE_CODE_MAKER_HPP

#include"CymStringOperation.hpp"
#include"CymNumConverter.hpp"
#include"CymTree.hpp"

namespace cym {

	class ICode {
	private:
		Tree icode_;
		Tree init_order_;
		Vector<Str> reserved_words_;
	public:
		ICode() : icode_(Tree::ObjectType{}), init_order_(Tree::ArrayType{}),reserved_words_{ u"var" } {

		}
		Vector<Pair<TokenKind,StrView>> getCallList(const StrView &expr,Map<StrView,Size> &priority) {
			Stack<StrView> stack;
			Vector<Pair<TokenKind, StrView>> call_list;
			for (auto token = takeToken(expr); !token.empty(); token = takeNextToken(expr, token)) {
				const TokenKind kind = getTokenKind(token,{u"+",u"*"}, reserved_words_);
				switch (kind) {
				case TokenKind::DECIMAL:
				case TokenKind::NUMBER:
				case TokenKind::STRINGLITERAL:
				case TokenKind::PARAM:
					call_list.emplaceBack(kind, token);
					break;
				case TokenKind::FUNC:
					call_list.emplaceBack(TokenKind::FUNC, token);
					for (const auto i : listArgs(token)) {
						call_list.pushBack(getCallList(i, priority));
					}
					break;
				case TokenKind::EXPRESSION:
					call_list.pushBack(getCallList(token.substr(1,token.size() - 2),priority));
					break;
				case TokenKind::INFIX:
					if (stack.empty()) {
						stack.push(token);
						break;
					}
					const auto top = priority[stack.top()];
					const auto current = priority[token];
					if (top < current) {
						for (auto seek_n = call_list.size() - stack.size() - 1; !stack.empty(); seek_n++) {
							call_list.insert(seek_n, makePair(TokenKind::INFIX, stack.top()));
							stack.pop();
						}
						stack.push(token);
					}
					else if (top >= current) {
						stack.push(token);
					}
				}
			}
			for (auto seek_n = call_list.size() - stack.size() - 1; !stack.empty(); seek_n++) {
				call_list.insert(seek_n, makePair(TokenKind::INFIX, stack.top()));
				stack.pop();
			}
			return call_list;
		}
		void compileLine(const StrView &code) {
			const auto token = takeToken(code);
			const TokenKind kind = getTokenKind(token, {}, reserved_words_);
			switch (kind) {
			case TokenKind::RESERVEDWORD:
				if (token == u"var") {
					const auto name = takeNextToken(code, token);
					const auto equal = takeNextToken(code, name);
					if (equal == u"=") {
						Tree constructor(Tree::ObjectType{});/*
						constructor.addWhenObject(u"Name", Str(name));
						constructor.addWhenObject(u"Args", );
						Tree define_param(Tree::ObjectType{});
						define_param.addWhenObject(u"Kind", u"DefineParam");
						define_param.addWhenObject(u"DefineParma",)
						init_order_.addWhenArray()*/
					}
					else {

					}

				}
				break;
			}
		}

	};


}

#endif