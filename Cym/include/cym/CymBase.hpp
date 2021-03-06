#ifndef CYM_BASE_HPP
#define CYM_BASE_HPP

#include<unordered_map>
#include<unordered_set>
#include<string>
#include<string_view>
#include<vector>
#include<memory>
#include<utility>
#include<variant>
#include<tuple>
#include<sstream>

namespace cym {
	using Size = std::size_t;
	using Int = std::int32_t;
	using Uint = std::uint32_t;
	template<class T,class Y>
	using Map = std::unordered_map<T, Y>;
	template<class T>
	using Set = std::unordered_set<T>;
	template<class T>
	using MultiSet = std::unordered_multiset<T>;
	template<class T>
	using Vector = std::vector<T>;
	using Str = std::u16string;
	using StrView = std::u16string_view;
	using Char = char16_t;
	template<class ...T>
	using Variant = std::variant<T...>;
	using Stream = std::basic_stringstream<char16_t>;

	enum class TokenKind {
		ERROR,
		RESERVEDWORD, // var,func,class
		VARIABLE, // param
		NUMBER, // 1 or 2 and so on.
		DECIMAL, // 1.14514
		FUNC, // func(), do()to() 
		STRINGLITERAL, // "string literal"

		EXPRESSION, // (a + b)
	};
}


#endif