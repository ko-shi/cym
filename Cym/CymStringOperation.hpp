#ifndef CYM_STRING_OPERATION_HPP
#define CYM_STRING_OPERATION_HPP

#include<string_view>
#include<string>
#include<deque>

#include"CymTCPair.hpp"


namespace cym {
	enum class TokenClass : std::uint8_t {
		ERROR,
		RESERVEDWORD, // var,func,class,lamda
		NAME, // name
		NUMBER, // 1 or 2 and so on.
		OPERATOR, // +,-,*,/,and so on.
		FUNC, // func()
		STRINGLITERAL, // "string literal"
		EXPRESSION, // (a + b)

	};
	
	constexpr char16_t* TokenClass_table[] = {
		u"ERROR",
		u"NAME",
		u"NUMBER",
		u"SIGN",
		u"FUNC",
		u"STRINGLITERAL",
		u"EXPRESSION"
	};


	template<class Int,class Str>
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
			auto old_num = func(str.substr(1));
			old_num.second *= -1;
			return old_num;
		}
		else {
			return func(str);
		}
	}
	template<class Str>
	Str takeWord(const Str &str) {
		auto itr = str.begin();
		while (itr != str.end() && *itr == u' ') {
			itr++;
		}
		const auto begin = itr;
		while (itr != str.end() && *itr != u' ') {
			itr++;
		}
		return str.substr(begin - str.begin(), itr - begin);
	}
	int gl_num_if_it_means_number_in_take_word = 0;
	/*
	The first of signs must be '(' ,and
	the second of signs must be ')'.
	*/
	template<class Str,class Container>
	Str takeWord(const Str &str,const Container &reserved_words,const Container &operators,TokenClass &meaning) {
		const auto compare = [&](auto itr, auto s) {
			std::size_t index = 0;
			for (auto i : s) {
				if (itr == str.end() || i != *itr) {
					return std::size_t{};// means 0
				}
				itr++;
				index++;
			}
			return index;
		};
		/* If isSign() == 0, it's not sign. If isSign() > 0, the returned value means the index + 1 of signs. */
		const auto isOperator = [&](const auto &itr) {
			for (const auto &i : operators) {
				if (const auto index = compare(itr, i);index != 0) {
					return index;
				}
			}
			return std::size_t{};// means 0
		};
		const auto pickUpBracket = [&](auto &itr,const auto &br_begin, const auto &br_end) {
			if (compare(itr, br_begin)) {
				std::size_t parenthesis_num = 0;
				do {
					if (itr != str.end()) {
						if (compare(itr, br_begin)) {
							parenthesis_num++;
						}
						else if (compare(itr, br_end)) {
							parenthesis_num--;
						}
						itr++;
					}
				} while (parenthesis_num != 0);
				return true;
			}
			return false;
		};
		auto itr = str.begin();
		while (itr != str.end() && *itr == u' ') {
			itr++;
		}
		if (itr == str.end()) {
			return Str{};
		}
		const auto begin = itr;
		if (pickUpBracket(itr, operators[0], operators[1])) {
			meaning = TokenClass::EXPRESSION;
			return str.substr(begin - str.begin(), itr - begin);
		}
		if (*itr == u'"') {
			do{
				itr++;
			}while (itr != str.end() && *itr != '"');
			itr++;
			meaning = TokenClass::STRINGLITERAL;
			return str.substr(begin - str.begin(), itr - begin);
		}
		if (const auto index = isOperator(itr);index != 0) {
			meaning = TokenClass::OPERATOR;
			return str.substr(begin - str.begin(), operators[index - 1].length());// index - 1 is for the definition of isSign()
		}
		while (itr != str.end() && *itr != u' ' && !isOperator(itr)) {
			itr++;
		}
		const auto content = str.substr(begin - str.begin(), itr - begin);
		if (pickUpBracket(itr, operators[0], operators[1])) {
			meaning = TokenClass::FUNC;
		}
		else {
			if (const auto result = toInteger<int>(content); result.first) {
				gl_num_if_it_means_number_in_take_word = result.second;
				meaning = TokenClass::NUMBER;
			}
			else {
				if (std::find(reserved_words.begin(), reserved_words.end(), content) == reserved_words.end()) {
					meaning = TokenClass::NAME;
				}
				else {
					meaning = TokenClass::RESERVEDWORD;
				}
			}
		}
		return content;
	}
	template<class Str>
	Str getRemainedStr(const Str &origin,const Str &last_word) {
		return origin.substr(last_word.data() + last_word.length() - origin.data());
	};
	template<class Str,class Container>
	Str seekToNextWord(const Str &origin, const Str &last_word,const Container &reserved_words,const Container &operators,TokenClass &kind) {
		return takeWord(getRemainedStr(origin, last_word),reserved_words,operators,kind);
	}

	/*
	The first of signs must be '(' ,and
	the second of signs must be ')'.
	PriorityFunc is bool(Str,Str),and have to return true in case of that left side has high priority.
	In case of right side has high priority, please return false.
	*/

	template<class Str, class Container, class PriorityFunc>
	Vector<Pair<TokenClass, Str>> convertToRPN(const Str &expression, const Container &operators, PriorityFunc &&func) {
		const auto isSign = [&operators](auto str) {
			for (const auto &i : operators) {
				if (i == str) {
					return true;
				}
			}
			return false;
		};

		Vector<Pair<TokenClass, Str>> buffer;
		std::deque<Pair<TokenClass, Str>> stack;
		TokenClass kind;
		for (auto str = takeWord(expression, operators, kind); str.length() != 0; str = seekToNextWord(expression, str, operators, kind)) {
			if (kind != TokenClass::OPERATOR) {
				buffer.emplaceBack(kind, str);
			}
			else {
				if (str == operators[1]) {// signs[1] is ')'
					while (stack.back().second != operators[0]) {// signs[0] is'('
						buffer.emplaceBack(stack.back());
						if (stack.size() == 0) {
							// TODO : error message
							// showtage of '(' ,or too much ')'
						}
						stack.pop_back();
					}
					stack.pop_back();
					continue;
				}
				else if (str == operators[0]) {// signs[0] is '('
					stack.emplace_back(TokenClass::OPERATOR, str);
					continue;
				}
				do {
					if (stack.empty()) {
						stack.emplace_back(TokenClass::OPERATOR, str);
						break;
					}
					if (func(str, stack.back().second)) {
						stack.emplace_back(TokenClass::OPERATOR, str);
						break;
					}
					else {
						buffer.emplaceBack(stack.back());
						stack.pop_back();
					}
				} while (1);
			}
		}
		for (const auto &i : stack) {
			buffer.emplaceBack(i);
		}
		return buffer;
	}

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
