#ifndef CYM_INTERMEDIATE_CODE_MAKER_HPP
#define CYM_INTERMEDIATE_CODE_MAKER_HPP

#include"CymStringOperation.hpp"
#include"CymNumConverter.hpp"
#include"CymTree.hpp"

namespace cym {

	class ICode {
	private:
	public:
		Tree icode_;
		Tree init_order_;
		Vector<Str> reserved_words_;
		Vector<StrView> infixes_;
		Map<StrView, Size> priority_;
	public:
		ICode() : icode_(Tree::ObjectType{}), init_order_(Tree::ArrayType{}), reserved_words_{ u"var" }
			, infixes_{ u"+",u"-",u"*",u"/" }, priority_{ { u"+",1 },{ u"-",1 },{ u"*",2 },{ u"/",2 } } {
				
		}
		Vector<Pair<TokenKind,StrView>> convertToPolishNotation(const StrView &expr) {
			Stack<StrView> stack;
			Vector<Pair<TokenKind, StrView>> call_list;
			for (auto token = takeToken(expr); !token.empty(); token = takeNextToken(expr, token)) {
				const TokenKind kind = getTokenKind(token,infixes_, reserved_words_);
				switch (kind) {
				case TokenKind::DECIMAL:
				case TokenKind::NUMBER:
				case TokenKind::STRINGLITERAL:
				case TokenKind::PARAM:
					call_list.emplaceBack(kind, token);
					break;
				case TokenKind::FUNC:
					call_list.emplaceBack(TokenKind::FUNC, token);
					break;
				case TokenKind::EXPRESSION:
					call_list.emplaceBack(TokenKind::EXPRESSION, token);
					break;
				case TokenKind::INFIX:
					if (stack.empty()) {
						stack.push(token);
						break;
					}
					const auto top = priority_[stack.top()];
					const auto current = priority_[token];
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
					break;
				}
			}
			for (auto seek_n = call_list.size() - stack.size() - 1; !stack.empty(); seek_n++) {
				call_list.insert(seek_n, makePair(TokenKind::INFIX, stack.top()));
				stack.pop();
			}
			for (Size i = 0; i < call_list.size(); i++) {
				if (call_list[i].first == TokenKind::FUNC) {
					Size index = 1;
					for (const auto arg : listArgs(call_list[i].second)) {
						call_list.insert(i + index,convertToPolishNotation(arg));
						index++;
					}
				}
				else if (call_list[i].first == TokenKind::EXPRESSION) {
					call_list.insert(i, convertToPolishNotation(call_list[i].second.substr(1, call_list[i].second.size() - 2)));
				}
			}
			return call_list;
		}
		Tree getParamTree(const Str &name) {
			Tree tree(Tree::ObjectType{});
			tree.addWhenObject(u"Type", Tree(Str(u"Param")));
			tree.addWhenObject(u"Name", Tree(name));
			return tree;
		}
		Tree convertPNToTree(Vector<Pair<TokenKind, StrView>>::iterator &itr, Size take_num) {
			Tree tree(Tree::ArrayType{});
			for (Size i = 0; i < take_num;i++,itr++) {
				const auto name = itr->second;
				switch (itr->first) {
				case TokenKind::NUMBER:
					tree.addWhenArray(Tree(toInt(name)));
					break;
				case TokenKind::DECIMAL:
					tree.addWhenArray(Tree(toDouble(name)));
					break;
				case TokenKind::STRINGLITERAL:
					tree.addWhenArray(Tree(Str(name)));
					break;
				case TokenKind::PARAM:{
					tree.addWhenArray(getParamTree(Str(name)));
					break;
				}
				case TokenKind::FUNC:
				case TokenKind::INFIX:{
					Tree func_obj(Tree::ObjectType{});
					func_obj.addWhenObject(u"Type", Tree(Str(u"Func")));
					const auto func_name = toFuncName(name, infixes_);
					func_obj.addWhenObject(u"Name", Tree(func_name));
					const auto arg_num = std::count(func_name.begin(), func_name.end(), u'@');
					auto itr_temp = itr + 1;
					func_obj.addWhenObject(u"Args", convertPNToTree(itr_temp, arg_num));
					itr = itr_temp - 1;
					tree.addWhenArray(std::move(func_obj));
					break;
				}
				}
			}
			return tree;
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
						const auto init_expr = takeNextToken(code, equal);
						auto pn = convertToPolishNotation(init_expr);
						auto pn_itr = pn.begin();
						auto expr_tree_arr = convertPNToTree(pn_itr, 1);
						auto &expr_tree = expr_tree_arr.get<Tree::ArrayType>()[0];
						
						Tree arg(Tree::ArrayType{});
						arg.addWhenArray(getParamTree(Str(name)));
						arg.addWhenArray(std::move(*expr_tree));
						Tree constructor(Tree::ObjectType{});
						constructor.addWhenObject(u"Name", Tree(Str(name)));
						constructor.addWhenObject(u"Args", std::move(arg));
						Tree define_param(Tree::ObjectType{});
						define_param.addWhenObject(u"Kind", Tree(Str(u"DefineParam")));
						define_param.addWhenObject(u"DefineParam", std::move(constructor));
						init_order_.addWhenArray(std::move(define_param));
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