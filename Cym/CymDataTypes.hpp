#ifndef CYM_DATA_TYPES_HPP
#define CYM_DATA_TYPES_HPP

#include<sstream>
#include<type_traits>
#include<unordered_map>


#include"CymVector.hpp"
#include"CymTCPair.hpp"



namespace cym {

	using Char = char16_t;
	using Str = std::u16string;
	using StrView = std::u16string_view;
	using Stream = std::basic_stringstream<char16_t>;
	template<class Key,class T>
	using Map = std::unordered_map<Key, T>;
	using Int = std::int32_t;
	using Uint = std::uint32_t;
	using Size = std::size_t;

	Pair<bool, StrView> toUint(const StrView &,Uint&);

	enum struct Endian {
		BIG,
		LITTLE
	};
	namespace impl {
		Endian check() {
			std::uint8_t buf[2];
			*static_cast<std::uint16_t*>(static_cast<void*>(buf)) = 0xAAFF;
			return buf[0] == 0xAA ? Endian::BIG : Endian::LITTLE;
		}
	}
	const Endian native_endian = impl::check();

	enum class TokenKind : std::uint8_t {
		ERROR,
		RESERVEDWORD, // var,func,class
		PARAM, // param
		NUMBER, // 1 or 2 and so on.
		DECIMAL, // 1.14514
		INFIX, // +,-,*,/,and so on.
		FUNC, // func(), do()to() 
		STRINGLITERAL, // "string literal"

		EXPRESSION, // (a + b) it's on the way of converting


		TYPEDETERMINED
	};

	constexpr Char* TokenClass_table[] = {
		u"error",
		u"reserved_word",
		u"param",
		u"number",
		u"decimal",
		u"sign",
		u"func",
		u"string_literal",
		u"expression",
	};
	struct WordInfo {
		TokenKind kind;
		Str name;
		std::pair<Size, Size> place;

		// option
		Size index_of_identifier;// index of Vector<FuncIdentifier or ParamIdentifier or ClassIdentifier>

	};
	/* type infering types */

	struct DataToInfer {
		enum {
			INFER
		};

	};


	struct FuncIdentifier {
		Str scope;
		Vector<Size> args;// size_t is indexnof type
		Str name;
	};
	bool operator==(const FuncIdentifier &l, const FuncIdentifier &r) {
		return l.scope == r.scope && l.args == r.args && l.name == r.name;
	}
	struct ParamIdentifier {
		Str scope;
		Str name;
		Size order;
		Str get()const {
			Stream str;
			str << order;
			return scope + u"/" + name + u"#" + str.str();
		}
	};
	ParamIdentifier makePIFromSingleStr(const Str &str) {// Make ClassIdentifier from single string. After '/' is name, before is name_space.
		const auto order_pos = str.rfind(u'/');
		const auto slash_pos = str.rfind(u'/');
		Uint temp;
		toUint(Str(str, order_pos), temp);
		return ParamIdentifier{ Str(str, 0, slash_pos),Str(str, slash_pos),temp };
	}
	bool operator==(const ParamIdentifier &l, const ParamIdentifier &r) {
		return l.scope == r.scope && l.name == r.name && l.order == r.order;
	}
	struct ForSameName {
		Str scope;
		Str name;
	};
	bool operator==(const ForSameName &l, const ForSameName &r) {
		return l.scope == r.scope && l.name == r.name;
	}
	struct ClassIdentifier {
		Str name_space;
		Str name;
		Str get()const {
			return name_space + u"/" + name;
		}
	};
	ClassIdentifier makeCIFromSingleStr(const Str &str) {// Make ClassIdentifier from single string. After '/' is name, before is name_space.
		const auto slash_pos = str.rfind(u'/');
		return ClassIdentifier{Str(str, 0, slash_pos),Str(str, slash_pos)};
	}

	bool operator==(const ClassIdentifier l, const ClassIdentifier r) {
		return l.name_space == r.name_space && l.name == r.name;
	}


	struct Command {
		enum Id {
			// Allocate memory of class(member value's memory). 
			ALLOC_CLASS, // index of Vector<ParamPos>,index of ClassInfo
			SUBSTITUTE, // index of Vector<ParamPos>,data
			PICK_UP, // index of Vector<ParamPos>,ignore
		} id;
		constexpr static char16_t* table[] = {
			u"ALLOC_CLASS",
			u"SUBSTITUTE",
		};
		union Data {
			Int i32[2];
			std::uint64_t u64;
		} data;
		Command(Id i, Uint d1, Uint d2) {
			id = i;
			data.i32[0] = d1;
			data.i32[1] = d2;
		}
	};

	struct FuncInfo {// This is in DoubleKeyMap.
		Size param_num;
		Size default_size;
		Vector<Command> command;
	};
	struct ClassInfo {// This is in DoubleKeyMap.
		Size size;// The hole size for initialization. This means only member_value
		Size num_of_member_param;// How many the parent FuncInstance should reserve Vector<ParamPos>.

		Vector<Size> param_indexes;// Member params. Vector of DoubleKeyMap of ClassInfo.
		Vector<Size> func_indexes;// Member functions. Vector of DoubleKeyMap of FuncInfo.
	};
	struct ParamPos { // This is like pointer. The memory is allocated by the parent's FuncInstance or ClassInstance.
		Size info_index;// This is index of the DoubleKeyMap of ClassInfo.
		Size begin_of_memory;// This is index of the member param of memory 
		Size length;// All Param are treated as array.
	};
	struct FuncInstance {
		Size info_index;// This is index of the DoubleKeyMap of FuncInfo.

		Vector<ParamPos> memory_use;// Vector's index is param index.
		Vector<std::uint8_t> memory;

		FuncInstance(Size index, Size param_num, Size default_memory_size){
			info_index = index;
			memory_use.resize(param_num);
			// In constructor, memory is served but not assigned.
			// assignment is runned in runtime.
			memory.reserve(default_memory_size);
		}
	};
	struct ClassInstance {
		Vector<ParamPos> member_value;
		Vector<Size> memory;

		std::vector<FuncInstance> member_func;
	};
}


namespace std {
	template<>
	struct hash<cym::FuncIdentifier> {
		size_t operator()(const cym::FuncIdentifier &i) const {
			return hash<cym::Str>()(i.scope) ^ hash<cym::Str>()(i.name);
		}
	};
	template<>
	struct hash<cym::ClassIdentifier> {
		size_t operator()(const cym::ClassIdentifier &c) const {
			return hash<cym::Str>()(c.name_space) ^ hash<cym::Str>()(c.name);
		}
	};
	template<>
	struct hash<cym::ParamIdentifier> {
		size_t operator()(const cym::ParamIdentifier &f) const {
			return hash<cym::Str>()(f.name) ^ hash<cym::Str>()(f.scope) + f.order;
		}
	};
	template<>
	struct hash<cym::ForSameName> {
		size_t operator()(const cym::ForSameName &f) const {
			return hash<cym::Str>()(f.name) ^ hash<cym::Str>()(f.scope);
		}
	};
}
#endif