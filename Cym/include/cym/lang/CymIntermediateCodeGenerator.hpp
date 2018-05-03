#ifndef CYM_INTERMEDIATE_CODE_GENERATOR_HPP
#define CYM_INTERMEDIATE_CODE_GENERATOR_HPP

#include<deque>

#include<cym/parser/CymStringOperation.hpp>
#include<cym/utils/CymNumConverter.hpp>
#include<cym/utils/CymTree.hpp>
#include<cym/utils/CymStack.hpp>

namespace cym {

	class ICode {
	private:
	public:
		enum ScopeKind {
			DEFINING_FUNC,
			WAITING_CONS_DEFINING,
			DEFINING_CLASS,
			DEFINING_METHOD
		};
		struct ScopeInfo {
			Tree* tree;
			ScopeKind kind;
			Set<StrView> restrictions;
			ScopeInfo(Tree *t, ScopeKind k, Set<StrView> && r = Set<StrView>{}) : tree(t), kind(k), restrictions(std::forward<Set<StrView>>(r)) {

			}
		};

		Tree icode_;
		Vector<Str> code_;
		Vector<ScopeInfo> scope_;
		Vector<Str> reserved_words_;
		Vector<StrView> infixes_;
		Map<StrView, Size> priority_;
		std::deque<Str> infixes_buffer_;
		const StrView single_indent_ = StrView(u"    ");
	public:
		ICode() : icode_(Tree::ObjectType{}), code_{},reserved_words_ { u"func", u"ret", u"cls" }
			, infixes_{ u"+",u"-",u"*",u"/" }, priority_{ { u"+",1 },{ u"-",1 },{ u"*",2 },{ u"/",2 } } {
			
			icode_.addWhenObject(u"DefinedFunc", Tree(Tree::ObjectType{}));
			icode_.addWhenObject(u"DefinedClass", Tree(Tree::ObjectType{}));
			icode_.addWhenObject(u"Order", Tree(Tree::ArrayType{}));

			scope_.emplace_back(
				&icode_,
				DEFINING_FUNC,
				Set<StrView>{u"var", u"let"}
			);
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
		bool isFollowing(StrView str) const{
			const auto token = takeWhile(str, [](Char c) {return c != u' '; });
			if (std::find(infixes_.begin(), infixes_.end(), token) != infixes_.end()) {
				return true;
			}
			if (!token.empty() && token[0] == u',') {
				return true;
			}
			return false;
		}
		void addLine(StrView code) {
			if (isFollowing(code)) {
				if (code_.empty()) {
					// コンパイルエラー
				}
				else {
					code_.back() += code;
				}
			}
			else {
				code_.emplace_back(code);
			}
		}
		void compileLine(StrView code,Size line) {
			Size indent_level = countStr(code, single_indent_);
			Size cur_scope_level = scope_.size() - 1;
			if (cur_scope_level < indent_level) {
				// TODO : エラー　定義された階層以上のインデント
				return;
			}
			// コメント
			if (const auto pos = code.find(u"//"); pos != code.npos) {
				code = code.substr(0, pos);
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
			auto &current_scope = scope_[indent_level].tree;
			auto &scope_kind = scope_[indent_level].kind;

			const auto token = takeToken(code,infixes_);
			const TokenKind kind = getTokenKind(token, infixes_, reserved_words_);

			/* 最初の単語が定義済みの型制約のとき */
			for (auto itr = scope_.rbegin(); itr != scope_.rend(); itr++) {
				const auto r = itr->restrictions;
				if (r.find(token) != r.end()) {
					caseDefineParam(code, token, current_scope,token);
					break;
				}
			}

			switch (kind) {
			case TokenKind::EXPRESSION:
				/* Constructor */
				if (!(scope_kind == ScopeKind::DEFINING_CLASS || scope_kind == ScopeKind::WAITING_CONS_DEFINING)) {
					break;
				}
				if (scope_kind == ScopeKind::WAITING_CONS_DEFINING) {
					caseDefineInitCons(code, token, current_scope, scope_kind,line);
				}
				else if (scope_kind == ScopeKind::DEFINING_CLASS) {
					Tree arg_restrictions(Tree::ArrayType{});
					Tree arg_names(Tree::ArrayType{});
					for (const auto arg : listArgs(token, infixes_)) {
						const auto restriction = takeToken(arg,infixes_);
						arg_restrictions.addWhenArray(Tree(Str(restriction)));

						const auto name = takeNextToken(code, restriction, infixes_);
						arg_names.addWhenArray(Tree(Str(name)));
					}
					Tree define_cons(Tree::ObjectType{});
					define_cons.addWhenObject(u"Restriction", std::move(arg_restrictions));
					define_cons.addWhenObject(u"ArgNames", std::move(arg_names));
					define_cons.addWhenObject(u"Order", Tree(Tree::ArrayType{}));
					current_scope->get<Tree::ObjectType>()[u"Cons"]->addWhenArray(std::move(define_cons));
					
					scope_.emplace_back(
						current_scope->get<Tree::ObjectType>()[u"Cons"]->get<Tree::ArrayType>().back().get()
						, DEFINING_METHOD
					);
				}

				break;
			case TokenKind::RESERVEDWORD:
				if (token == u"func") {
					caseDefineFunc(code, token, current_scope,line);
				}
				else if (token == u"ret") {
					caseReturn(code, token, current_scope);
				}
				else if (token == u"cls") {
					caseDefineClass(code, token, current_scope,line);
				}
				break;
			default:
			{
				if (scope_kind == ScopeKind::DEFINING_CLASS) {
					//クラス定義での文
					//コンパイルエラー
					return;
				}
				Tree tree(getExprTree(code));
				tree.get<Tree::ObjectType>()[u"Kind"]->get<Str>() = u"Call";
				current_scope->get<Tree::ObjectType>()[u"Order"]->addWhenArray(std::move(tree));
			}
			}
		}
		void caseDefineParam(StrView code, StrView token, Tree * &current_scope, StrView restriction) {
			const auto name = takeNextToken(code, token, infixes_);
			const auto equal = takeNextToken(code, name, infixes_);
			if (equal == u"=") {
				const auto init_expr = getRemainedStr(code, equal);

				Tree arg(Tree::ArrayType{});
				arg.addWhenArray(getParamTree(Str(name)));
				arg.addWhenArray(getExprTree(init_expr));
				Tree define_param(Tree::ObjectType{});
				define_param.addWhenObject(u"Kind", Tree(Str(u"DefineParam")));
				define_param.addWhenObject(u"Name", Tree(Str(name)));
				define_param.addWhenObject(u"Cons", std::move(arg));
				define_param.addWhenObject(u"Restriction", Tree(Str(restriction)));
				current_scope->get<Tree::ObjectType>()[u"Order"]->addWhenArray(std::move(define_param));
			}
			else {

			}
		}
		void caseReturn(StrView code,StrView token,Tree * &current_scope) {
			const auto ret_expr = getRemainedStr(code, token);
			Tree tree(getExprTree(ret_expr));
			tree.get<Tree::ObjectType>()[u"Kind"]->get<Str>() = u"ReturnFunc";
			current_scope->get<Tree::ObjectType>()[u"Order"]->addWhenArray(std::move(tree));
		}
		void caseDefineFunc(StrView code,StrView token,Tree * &current_scope,Size line) {
			const auto func_decl = takeNextToken(code, token, infixes_);
			const Str func_name = toFuncName(func_decl, infixes_);
			if (getTokenKind(func_decl, infixes_, reserved_words_, &func_name) == TokenKind::FUNC) {
				Tree arg_names(Tree::ArrayType{});
				Tree arg_restrictions(Tree::ArrayType{});
				for (const auto &arg : listArgs(func_decl, infixes_)) {
					const auto type_restriction = takeToken(arg, infixes_);
					arg_restrictions.addWhenArray(Tree(Str(type_restriction)));
					/* TODO : type_restriction */
					const auto name = takeNextToken(code, type_restriction, infixes_);
					arg_names.addWhenArray(Tree(Str(name)));
				}
				Tree define_arg(Tree::ObjectType{});
				define_arg.addWhenObject(u"Kind", Tree(Str(u"PickUpArg")));
				define_arg.addWhenObject(u"Num", Tree(static_cast<Int>(arg_names.get<Tree::ArrayType>().size())));

				Tree order(Tree::ArrayType{});
				order.addWhenArray(std::move(define_arg));


				Tree define_func(Tree::ObjectType{});
				define_func.addWhenObject(u"Kind", Tree(Str(u"DefineFunc")));
				define_func.addWhenObject(u"ArgNames", std::move(arg_names));
				define_func.addWhenObject(u"Restriction", std::move(arg_restrictions));
				define_func.addWhenObject(u"Order", std::move(order));
				define_func.addWhenObject(u"DefinedClass", Tree(Tree::ObjectType{}));
				define_func.addWhenObject(u"DefinedFunc", Tree(Tree::ObjectType{}));

				current_scope->get<Tree::ObjectType>()[u"DefinedFunc"];

				auto &place_to_insert = current_scope->get<Tree::ObjectType>()[u"DefinedFunc"];
				auto pos_of_the_name = place_to_insert->get<Tree::ObjectType>().find(func_name);
				if (pos_of_the_name == place_to_insert->get<Tree::ObjectType>().end()) {
					place_to_insert->addWhenObject(func_name, Tree(Tree::ArrayType{}));
					place_to_insert->get<Tree::ObjectType>()[func_name]->addWhenArray(std::move(define_func));
				}
				else {
					pos_of_the_name->second->addWhenArray(std::move(define_func));
				}

				scope_.emplace_back(
					place_to_insert->get<Tree::ObjectType>()[func_name]->get<Tree::ArrayType>().back().get()
					, DEFINING_FUNC
				);
				scanClsDefinition(line + 1);
			}
		}
		void caseDefineClass(StrView code, StrView token, Tree * &current_scope,Size line) {
			const auto name = takeNextToken(code, token, infixes_);
			Tree define_class(Tree::ObjectType{});
			define_class.addWhenObject(u"Kind", Tree(Str(u"DefineClass")));
			define_class.addWhenObject(u"Member", Tree(Tree::ArrayType{}));
			define_class.addWhenObject(u"Restriction", Tree(Tree::ArrayType{}));
			define_class.addWhenObject(u"Method", Tree(Tree::ObjectType{}));

			const Str cls_name = Str(name);

			auto &place_to_insert = current_scope->get<Tree::ObjectType>()[u"DefinedClass"];
			auto pos_of_the_name = place_to_insert->get<Tree::ObjectType>().find(cls_name);
			if (pos_of_the_name == place_to_insert->get<Tree::ObjectType>().end()) {
				place_to_insert->addWhenObject(cls_name, std::move(define_class));
			}
			else {
				// 同名クラスの複数定義
				// コンパイルエラー
				return;
			}
			scope_.emplace_back(
				current_scope->get<Tree::ObjectType>()[u"DefinedClass"]->get<Tree::ObjectType>()[Str(name)].get(),
				WAITING_CONS_DEFINING
			);
			scanClsDefinition(line + 1);
		}
		void caseDefineInitCons(StrView code,StrView token,Tree * &current_scope,ScopeKind &scope_kind,Size line) {
			Tree restrictions(Tree::ArrayType{});
			Tree arg_names(Tree::ArrayType{});
			for (const auto &arg : listArgs(token, infixes_)) {
				const auto type_restriction = takeToken(arg, infixes_);
				current_scope->get<Tree::ObjectType>()[u"Restriction"]->addWhenArray(Tree(Str(type_restriction)));
				restrictions.addWhenArray(Tree(Str(type_restriction)));

				const auto name = takeNextToken(code, type_restriction, infixes_);
				current_scope->get<Tree::ObjectType>()[u"Member"]->addWhenArray(Tree(Str(name)));
				arg_names.addWhenArray(Tree(Str(name)));
			}
			Tree cons_obj(Tree::ObjectType{});
			cons_obj.addWhenObject(u"Restriction", std::move(restrictions));
			cons_obj.addWhenObject(u"ArgNames", std::move(arg_names));
			cons_obj.addWhenObject(u"Order", Tree(Tree::ArrayType{}));

			current_scope->get<Tree::ObjectType>()[u"Method"]->addWhenObject(u"Cons", Tree(Tree::ArrayType{}));
			current_scope->get<Tree::ObjectType>()[u"Method"]->get<Tree::ObjectType>()[u"Cons"]->addWhenArray(std::move(cons_obj));


			current_scope = current_scope->get<Tree::ObjectType>()[u"Method"].get();
			scope_kind = ScopeKind::DEFINING_CLASS;
			scope_.emplace_back(
				current_scope->get<Tree::ObjectType>()[u"Cons"]->get<Tree::ArrayType>().back()->get<Tree::ObjectType>()[u"Order"].get()
				, DEFINING_METHOD
			);
			scanClsDefinition(line + 1);

		}
		void scanClsDefinition(Size line) {
			for (auto i = line; i < code_.size();i++) {
				const auto str = code_[i];
				Size indent_level = countStr(str, single_indent_);
				Size cur_indent_level = scope_.size() - 1;
				if (indent_level == cur_indent_level) {
					const auto token = takeWhile(deleteSpace(str), [](Char c) {return c != u' '; });
					if (token == u"cls") {
						const auto name = takeNextToken(str, token, infixes_);
						scope_.back().restrictions.emplace(name);
					}
				}
				else if (indent_level < cur_indent_level) {
					break;
				}
			}
		}

		void compile() {
			// タブ文字をスペースに置き換え
			for (auto &&s : code_) {
				for (auto itr = s.begin(); itr != s.end();itr++) {
					if (*itr == u'\t') {
						s.replace(itr, itr + 1, single_indent_);
					}
				}
			}

			scanClsDefinition(0);

			Size index = 0;
			for (const auto &s : code_) {
				compileLine(StrView(s),index);
				index++;
			}
		}
	};


}

#endif