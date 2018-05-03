#ifndef CYM_STRING_OPERATION_HPP
#define CYM_STRING_OPERATION_HPP

#include<string_view>
#include<string>
#include<deque>
#include<algorithm>
#include<variant>

#include<cym/CymBase.hpp>
#include<cym/utils/CymStack.hpp>
#include<cym/utils/CymTCPair.hpp>

#ifdef ERROR
#	undef ERROR
#endif


namespace cym {
	Pair<std::variant<double,Int>, StrView> getNumKind(const StrView &str, TokenKind &kind);

	template<class F>
	StrView takeWhile(const StrView &str, F &&f) {
		// functional writiing is too slow!
		Size index = 0;
		for (auto i : str) {
			if (!f(i)) {
				break;
			}
			index++;
		}
		return str.substr(0, index);
	}
	StrView getBlock(const StrView &str) {
		const auto brackets = Vector<Pair<Char, Char>>{ { u'(',u')' },{ u'[',u']' },{ u'<',u'>' },{ u'{',u'}' } };
		Stack<Char> stack;
		Size index = 0;
		if (str.empty())return str;
		if (str[0] == u'"') {
			return str.substr(0, takeWhile(str.substr(1), [](auto c) {return c != u'"'; }).size() + 2);
		}
		for (const auto &c : str) {
			for (const auto &b : brackets) {
				if (b.first == c) {
					stack.push(b.second);
				}
				if (b.second == c) {
					if (stack.top() == b.second) {
						stack.pop();
					}
					else {
						return StrView{};
					}
				}
			}

			index++;
			if (stack.empty()) {
				break;
			}
		}
		return str.substr(0, index);
	}
	StrView bracketContent(const StrView &str) {
		return str.size() <= 1 ? str : str.substr(1, str.size() - 2);
	}
	StrView getRemainedStr(const StrView &origin, const StrView &last_word) {
		return origin.substr(last_word.data() + last_word.length() - origin.data());
	}
	StrView expandTail(const StrView &origin, const StrView &str,Size expand_num) {
		return origin.data() + origin.size() < str.data() + str.size() + expand_num ?
			str
			: StrView(str.data(), str.size() + expand_num);
	}
	StrView rangeOf(const StrView &begin, const StrView &end) {
		return StrView(begin.data(), (end.data() + end.size()) - begin.data());
	}
	bool isSub(const StrView &str, const StrView &sub) {
		return str.data() <= sub.data() && str.data() + str.size() >= sub.data() + str.size();
	}
	StrView deleteSpace(const StrView &str) {
		return getRemainedStr(str, takeWhile(str, [](auto c) {return c == u' '; }));
	}
	StrView takeToken(const StrView &str,const Vector<StrView> &infixes,bool can_delete_front_space = true) {

		const auto specials = Vector<Char>{
			u'(' ,u')',u'[' ,u']',u'{',u'}' ,u'"',u'<',u'>',
			u'+' ,u'-' ,u'*' ,u'/' ,
			u'.',
			u',' ,u' ',u';'
		};
		const auto takeWhileName = [&](StrView word) {return takeWhile(word, [&](auto c) {return std::none_of(specials.begin(), specials.end(), [&](auto s) {return s == c; }); }); };
		const auto word = can_delete_front_space ? 
			deleteSpace(str)
			: str;
		const auto until_space = takeWhile(word, [](Char c) {return c != u' '; });
		if (std::find(infixes.begin(), infixes.end(), until_space) != infixes.end()) {
			return until_space;
		}

		const auto name = takeWhileName(word);
		const auto sign_part = getRemainedStr(str, name);

		if (sign_part.empty()) {
			return name;
		}
		switch (sign_part[0]) {
		case u'+':
		case u'-':
		case u'*':
		case u'/':
			if (name.empty()) {
				return sign_part.substr(0, 1);
			}
			else {
				return name;
			}
		case u'(': {
			const auto bracket_part = getBlock(sign_part);
			const auto next = getRemainedStr(str, bracket_part);
			if (next.empty() || std::any_of(specials.begin() + 1/* + 1 for except '(' */, specials.end(), [&](Char c) {return c == next[0]; })) {
				return rangeOf(name,bracket_part);
			}
			else {
				return rangeOf(name, takeToken(next, false));// rangeOf is like operator+
			}
		}
		case u',':
		case u' ':
			return name;
		case u'"':
		case u'[':
			if (name.empty()) {
				return getBlock(sign_part);
			}
			else {
				return name;
			}
		}
		return name;
	}
	StrView takeNextToken(const StrView &origin,const StrView &last_token,const Vector<StrView> &infixes) {
		return takeToken(getRemainedStr(origin, last_token),infixes);
	}
	StrView takeExpression(const StrView &str,const Vector<StrView> &infixes) {
		const auto word = takeToken(str,infixes);
		const auto next_word = getRemainedStr(str, takeWhile(getRemainedStr(str, word), [](auto c) {return c == u' '; }));
		return word.empty() || next_word.empty() || next_word[0] == u',' ? word : rangeOf(word, takeExpression(next_word,infixes));
	}
	Str repeat(const Str &str, Size n, Str b = u"") {
		return n == 0 ? b : repeat(str, n - 1, b + str);
	}
	// 114514,yaju,MUR => @,@,@
	Str replaceExpression(const StrView &str,const Vector<StrView> &infixes,int expr_num = 0) {
		const auto expr = takeExpression(str,infixes);
		const auto next = getRemainedStr(str, takeWhile(getRemainedStr(str, expr), [](auto c) {return c == u' ' || c == u','; }));
		return next.empty() ? repeat(u"@",expr_num + 1): replaceExpression(next,infixes,expr_num + 1);
	}
	// This function's arg must be deleted the initial spaces.
	Str toFuncName(const StrView &token, const Vector<StrView> &infixes) {

		if (std::find(infixes.begin(), infixes.end(), token) != infixes.end()) {
			return Str(u"@") + Str(token) + u"@";
		}


		const auto specials = Vector<Char>{
			u'(' ,u'[' ,u'{' ,u'"',u'<',
			u'+' ,u'-' ,u'*' ,u'/' ,
			u'.',
			u',' ,u' '
		};
		const auto takeWhileName = [&](StrView word) {return takeWhile(word, [&](auto c) {return std::none_of(specials.begin(), specials.end(), [&](auto s) {return s == c; }); }); };
		const auto name_part = takeWhileName(token);
		const auto bracket_part = getBlock(getRemainedStr(token, name_part));
		if (bracket_part.empty() || bracket_part[0] != u'(') {
			return Str(name_part);
		}
		const Str args_replaced =
			bracket_part.empty() ?
			Str()
			: replaceExpression(bracket_part.substr(1, bracket_part.size() - 2),infixes);
		const auto next = getRemainedStr(token, bracket_part);

		return Str(name_part) + args_replaced + toFuncName(next, infixes);
	}
	// This function's arg must be deleted the initial spaces.
	Vector<StrView> listArgs(const StrView &func,const Vector<StrView> &infixes) {
		const auto specials = Vector<Char>{
			u'(' ,u'[' ,u'{' ,u'"',u'<',
			u'+' ,u'-' ,u'*' ,u'/' ,
			u'.',
			u',' ,u' '
		};
		const auto takeWhileName = [&](StrView word) {return takeWhile(word, [&](auto c) {return std::none_of(specials.begin(), specials.end(), [&](auto s) {return s == c; }); }); };
		const auto name_part = takeWhileName(func);
		const auto bracket_part = getBlock(getRemainedStr(func, name_part));
		if (bracket_part.empty() || bracket_part[0] != u'(') {
			return Vector<StrView>{};
		}
		const auto bracket_content = bracketContent(bracket_part);
		Vector<StrView> list;
		for (StrView expr = takeExpression(bracket_content,infixes); !expr.empty();
			expr = takeExpression(getRemainedStr(bracket_content,expandTail(bracket_content,expr,1)),infixes)) {
			list.emplaceBack(expr);
		}
		const auto next = getRemainedStr(func, bracket_part);
		list.pushBack(listArgs(next,infixes));
		return list;
	}
	TokenKind getTokenKind(const StrView &token/* please convert to StrView */, const Vector<StrView> &infixes,const Vector<Str> &reserved_words,const Str *func_name = nullptr) {
		TokenKind kind;
		getNumKind(token, kind);
		if (kind != TokenKind::ERROR) {
			return kind;
		}
		if (token.size() >= 2 && token[0] == u'"' && token.back() == u'"') {
			return TokenKind::STRINGLITERAL;
		}
		if (std::find(reserved_words.begin(), reserved_words.end(), Str(token)) != reserved_words.end()) {
			return TokenKind::RESERVEDWORD;
		}
		if (std::find(infixes.begin(), infixes.end(), Str(token)) != infixes.end()) {
			return TokenKind::INFIX;
		}

		const auto lastJunction = [](const auto &s) {

			if (s.find(u'@') == s.npos) {
				return TokenKind::PARAM;
			}
			else if (std::all_of(s.begin(), s.end(), [](Char c) {return c == u'@'; })) {
				return TokenKind::EXPRESSION;
			}
			return TokenKind::FUNC;
		};

		return func_name ? lastJunction(*func_name) : lastJunction(toFuncName(token,infixes));
	}

	Size countStr(const StrView &str, const StrView &contrast) {
		Size i = 0;
		for (auto token = str; token.substr(0, contrast.size()) == contrast;
			token = getRemainedStr(str, token.substr(0, contrast.size())), i++) {

		}
		return i;
	}
}
#endif