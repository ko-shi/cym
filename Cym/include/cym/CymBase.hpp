#ifndef CYM_BASE_HPP
#define CYM_BASE_HPP

#include<unordered_map>
#include<string>
#include<string_view>
#include<sstream>

namespace cym {
	using Size = std::size_t;
	using Int = std::int32_t;
	using Uint = std::uint32_t;
	template<class T,class Y>
	using Map = std::unordered_map<T, Y>;
	using Str = std::u16string;
	using StrView = std::u16string_view;
	using Char = char16_t;
	using Stream = std::basic_stringstream<char16_t>;

	enum class TokenKind {
		ERROR,
		RESERVEDWORD, // var,func,class
		PARAM, // param
		NUMBER, // 1 or 2 and so on.
		DECIMAL, // 1.14514
		INFIX, // +,-,*,/,and so on.
		FUNC, // func(), do()to() 
		STRINGLITERAL, // "string literal"

		EXPRESSION, // (a + b) it's on the way of converting
	};
}


#endif