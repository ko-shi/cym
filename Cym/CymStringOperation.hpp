#ifndef CYM_STRING_OPERATION_HPP
#define CYM_STRING_OPERATION_HPP

#include<string_view>
#include<string>
#include<deque>

#include"CymTCPair.hpp"
#include"CymDataTypes.hpp"


namespace cym {
	


	template<class Int>
	std::pair<bool/* succeed */, Int> toInteger(const Str &str) {
		const auto func = [](const Str &s) {
			Int num = 0;
			for (const auto i : s) {
				if (u'0' <= i && i <= u'9') {
					num *= 10;
					num += (i - u'0');
				}
				else {
					return std::make_pair(false, num);
				}
			}
			return std::make_pair(true, num);
		};
		if (str.front() == u'-') {
			if (std::is_unsigned_v<Int>) {
				return func(str.substr(1));
			}
			auto old_num = func(str.substr(1));
			old_num.second *= -1;
			return old_num;
		}
		else {
			return func(str);
		}
	}
	template<class Container>
	StrView getBlock(const StrView &str,const Container &brackets) {
		Stack<Char> stack;
		std::size_t index = 0;
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
						return u"Error";
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
	template<class F>
	StrView takeWhile(const StrView &str, F &&f) {
		// functional writiing is too slow!
		std::size_t index = 0;
		for (auto i : str) {
			if (!f(i)) {
				break;
			}
			index++;
		}
		return str.substr(0, index);
	}
	StrView getRemainedStr(const StrView &origin, const StrView &last_word) {
		return origin.substr(last_word.data() + last_word.length() - origin.data());
	}
	StrView rangeOf(const StrView &begin, const StrView &end) {
		const auto a = StrView(begin.data(), (end.data() + end.size()) - begin.data());
		return StrView(begin.data(), (end.data() + end.size()) - begin.data());
	}
	StrView deleteSpace(const StrView &str) {
		return getRemainedStr(str, takeWhile(str, [](auto c) {return c == u' '; }));
	}
	StrView takeArg(const StrView &str,bool can_delete_front_space = true) {
		const auto specials = Vector<Char>{
			u'(' ,u'[' ,u'{' ,u'"',u'<',
			u'+' ,u'-' ,u'*' ,u'/' ,
			u'.',
			u',' ,u' '
		};
		const auto brackets = Vector<Pair<Char, Char>>{ { u'(',u')' },{ u'[',u']' },{ u'<',u'>' },{ u'{',u'}' },{ u'"',u'"' } };
		const auto takeWhileName = [&](StrView word) {return takeWhile(word, [&](auto c) {return std::none_of(specials.begin(), specials.end(), [&](auto s) {return s == c; }); }); };
		const auto word = can_delete_front_space ? 
			deleteSpace(str)
			: str;
		const auto name = takeWhileName(word);
		const auto sign_part = getRemainedStr(str, name);

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
			const auto bracket_part = getBlock(sign_part, brackets);
			const auto next = getRemainedStr(str, bracket_part);
			if (std::any_of(specials.begin() + 1/* + 1 for except '(' */, specials.end(), [&](Char c) {return c == next[0]; })) {
				return rangeOf(name,bracket_part);
			}
			else {
				return rangeOf(name, takeArg(next, false));// rangeOf is like operator+
			}
		}
		case u',':
		case u' ':
			return name;
		}
		return name;
	}

	StrView takeExpression(const StrView &str) {
		const auto word = takeArg(str);
		const auto next_word = getRemainedStr(str, takeWhile(getRemainedStr(str, word), [](auto c) {return c == u' '; }));
		return next_word.empty() || next_word[0] == u',' ? word : rangeOf(word, takeExpression(next_word));
	}

	Str repeat(const Str &str, std::size_t n, Str b = u"") {
		return n == 0 ? b.pop_back(),b : repeat(str, n - 1, b + str + u',');
	}
	// 114514,yaju,MUR => @,@,@
	Str replaceExpression(const StrView &str,int expr_num = 0) {
		const auto expr = takeExpression(str);
		const auto next = getRemainedStr(str, takeWhile(getRemainedStr(str, expr), [](auto c) {return c == u' ' || c == u','; }));
		return next.empty() ? repeat(u"@",expr_num + 1): replaceExpression(next,expr_num + 1);
	}

	Str toFuncName(const StrView &str) {
		const auto specials = Vector<Char>{
			u'(' ,u'[' ,u'{' ,u'"',u'<',
			u'+' ,u'-' ,u'*' ,u'/' ,
			u'.',
			u',' ,u' '
		};
		const auto brackets = Vector<Pair<Char, Char>>{ { u'(',u')' },{ u'[',u']' },{ u'<',u'>' },{ u'{',u'}' },{ u'"',u'"' } };
		const auto takeWhileName = [&](StrView word) {return takeWhile(word, [&](auto c) {return std::none_of(specials.begin(), specials.end(), [&](auto s) {return s == c; }); }); };
		const auto name_part = takeWhileName(str);
		const auto bracket_part = getBlock(getRemainedStr(str, name_part), brackets);
		

		return u"";
	}

	int gl_num_if_it_means_number_in_take_word = 0;
	/*
	The first of signs must be '(' ,and
	the second of signs must be ')'.
	*/

	/*
	The first of signs must be '(' ,and
	the second of signs must be ')'.
	PriorityFunc is bool(Str,Str),and have to return true in case of that left side has high priority.
	In case of right side has high priority, please return false.
	*/


	template<class Str,class T>
	Str showRPN(const T &rpn) {
		Str str;
		for (const auto &i : rpn) {
			str += Str(TokenClass_table[static_cast<std::size_t>(i.first)]) + Str(u",") + Str(i.second) + Str(u"\n");
		}
		return str;
	}
}
#endif
