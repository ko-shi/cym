#ifndef CYM_INTERMEDIATE_CODE_MAKER_HPP
#define CYM_INTERMEDIATE_CODE_MAKER_HPP

#include<deque>

#include"CymStringOperation.hpp"
#include"CymNumConverter.hpp"
#include"CymTree.hpp"

namespace cym {

	class ICode {
	private:
	public:
		Tree icode_;
		Vector<Tree*> scope_;
		Vector<Str> reserved_words_;
		Vector<StrView> infixes_;
		Map<StrView, Size> priority_;
		std::deque<Str> infixes_buffer_;
		const StrView single_indent_ = StrView(u"    ");
	public:
		ICode() : icode_(Tree::ArrayType{}), reserved_words_{ u"var",u"func",u"ret" }
			, infixes_{ u"+",u"-",u"*",u"/" }, priority_{ { u"+",1 },{ u"-",1 },{ u"*",2 },{ u"/",2 } } {

			Tree init(Tree::ObjectType{});
			init.addWhenObject(u"Kind", Tree(Str(u"DefineFunc")));
			init.addWhenObject(u"Name", Tree(Str(u"init@")));
			init.addWhenObject(u"Order", Tree(Tree::ArrayType{}));
			icode_.addWhenArray(std::move(init));
			scope_.pushBack(icode_.get<Tree::ArrayType>().back()->get<Tree::ObjectType>()[u"Order"].get());
			//scope_.pushBack();
		}
		void addInfix(Str &&infix_name) {
			infixes_buffer_.emplace_back(std::forward<Str>(infix_name));
			infixes_.emplaceBack(StrView(infixes_buffer_.back()));
		}
		Vector<Pair<TokenKind,StrView>> convertToPolishNotation(const StrView &expr) {
			Stack<StrView> stack;
			Vector<Pair<TokenKind, StrView>> call_list;
			for (auto token = takeToken(expr,infixes_); !token.empty(); token = takeNextToken(expr, token,infixes_)) {
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
					for (const auto arg : listArgs(call_list[i].second,infixes_)) {
						call_list.insert(i + index,convertToPolishNotation(arg));
						index++;
					}
				}
				else if (call_list[i].first == TokenKind::EXPRESSION) {
					call_list.insert(i, convertToPolishNotation(bracketContent(call_list[i].second)));
				}
			}
			return call_list;
		}
		Tree getParamTree(const Str &name) const{
			Tree tree(Tree::ObjectType{});
			tree.addWhenObject(u"Kind", Tree(Str(u"Param")));
			tree.addWhenObject(u"Name", Tree(name));
			return tree;
		}
		Tree convertPNToTree(Vector<Pair<TokenKind, StrView>>::iterator &itr, Size take_num) const{
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
					tree.addWhenArray(Tree(Str(name.substr(1,name.size() - 2))));
					break;
				case TokenKind::PARAM:{
					tree.addWhenArray(getParamTree(Str(name)));
					break;
				}
				case TokenKind::FUNC:
				case TokenKind::INFIX:{
					Tree func_obj(Tree::ObjectType{});
					func_obj.addWhenObject(u"Kind", Tree(Str(u"Func")));
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
		Tree getExprTree(const StrView &expr) {
			auto pn = convertToPolishNotation(expr);
			auto pn_itr = pn.begin();
			auto expr_tree_arr = convertPNToTree(pn_itr, 1);
			return Tree(std::move(*expr_tree_arr.get<Tree::ArrayType>()[0]));
		}
		void compileLine(const StrView &code) {
			Size indent_level = countStr(code, single_indent_);
			Size cur_scope_level = scope_.size() - 1;
			if (cur_scope_level < indent_level) {
				// TODO : エラー　定義された階層以上のインデント
				return;
			}
			if (cur_scope_level > indent_level) {
				if (getRemainedStr(code, takeWhile(code, [](Char c) {return c == u' '; })).empty()) {
					return;// Empty line
				}
				else {
					while (scope_.size() - 1 > indent_level) {
						scope_.popBack();
					}
				}
			}
			const auto &current_scope = scope_[indent_level];


			const auto token = takeToken(code,infixes_);
			const TokenKind kind = getTokenKind(token, infixes_, reserved_words_);
			switch (kind) {
			case TokenKind::RESERVEDWORD:
				if (token == u"var") {
					const auto name = takeNextToken(code, token,infixes_);
					const auto equal = takeNextToken(code, name,infixes_);
					if (equal == u"=") {
						const auto init_expr = getRemainedStr(code, equal);

						Tree arg(Tree::ArrayType{});
						arg.addWhenArray(getParamTree(Str(name)));
						arg.addWhenArray(getExprTree(init_expr));
						Tree define_param(Tree::ObjectType{});
						define_param.addWhenObject(u"Kind", Tree(Str(u"DefineParam")));
						define_param.addWhenObject(u"Name", Tree(Str(name)));
						define_param.addWhenObject(u"Cons", std::move(arg));
						current_scope->addWhenArray(std::move(define_param));
					}
					else {

					}

				}
				else if (token == u"func") {
					const auto func_decl = takeNextToken(code, token,infixes_);
					const Str func_name = toFuncName(func_decl, infixes_);
					if (getTokenKind(func_decl, infixes_, reserved_words_,&func_name) == TokenKind::FUNC) {
						Tree arg_names(Tree::ArrayType{});
						for (const auto &arg : listArgs(func_decl,infixes_)) {
							const auto type_restriction = takeToken(arg,infixes_);
							/* TODO : type_restriction */
							const auto name = takeNextToken(code, type_restriction,infixes_);
							arg_names.addWhenArray(Tree(Str(name)));
						}
						Tree define_arg(Tree::ObjectType{});
						define_arg.addWhenObject(u"Kind", Tree(Str(u"DefineArg")));
						define_arg.addWhenObject(u"Name", std::move(arg_names));

						Tree order(Tree::ArrayType{});
						order.addWhenArray(std::move(define_arg));

						
						Tree define_func(Tree::ObjectType{});
						define_func.addWhenObject(u"Kind", Tree(Str(u"DefineFunc")));
						define_func.addWhenObject(u"Name",Tree(func_name));
						define_func.addWhenObject(u"Order", std::move(order));

						current_scope->addWhenArray(std::move(define_func));
						scope_.pushBack(current_scope->get<Tree::ArrayType>().back()->get<Tree::ObjectType>()[u"Order"].get());

					}
					else {

					}
				}
				else if (token == u"ret") {
					const auto ret_expr = getRemainedStr(code, token);
					Tree tree(getExprTree(ret_expr));
					tree.get<Tree::ObjectType>()[u"Kind"]->get<Str>() = u"ReturnFunc";
					current_scope->addWhenArray(std::move(tree));
				}
				break;
			default:
			{
				Tree tree(getExprTree(code));
				tree.get<Tree::ObjectType>()[u"Kind"]->get<Str>() = u"Call";
				current_scope->addWhenArray(std::move(tree));
			}
			}
		}
	};


}

#endif