#ifndef CYM_DATA_TYPES_HPP
#define CYM_DATA_TYPES_HPP

#include<sstream>
#include<type_traits>

#include"CymVector.hpp"
#include"CymStringOperation.hpp"

namespace cym {

	using Str = std::u16string;
	using StrView = std::u16string_view;
	using Stream = std::basic_stringstream<char16_t>;

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

	struct WordInfo {
		TokenClass kind;
		Str name;
		std::pair<std::size_t, std::size_t> place;

		// option
		std::size_t index_of_identifier;// index of Vector<FuncIdentifier or ParamIdentifier or ClassIdentifier>

	};
	/* type infering types */

	struct DataToInfer {
		enum {
			INFER
		};

	};


	struct FuncIdentifier {
		Str scope;
		Vector<std::size_t> args;// size_t is indexnof type
		Str name;
	};
	bool operator==(const FuncIdentifier &l, const FuncIdentifier &r) {
		return l.scope == r.scope && l.args == r.args && l.name == r.name;
	}
	struct ParamIdentifier {
		Str scope;
		Str name;
		std::size_t order;
		Str get()const {
			Stream str;
			str << order;
			return scope + u"/" + name + u"#" + str.str();
		}
	};
	ParamIdentifier makePIFromSingleStr(const Str &str) {// Make ClassIdentifier from single string. After '/' is name, before is name_space.
		const auto order_pos = str.rfind(u'/');
		const auto slash_pos = str.rfind(u'/');
		return ParamIdentifier{ Str(str, 0, slash_pos),Str(str, slash_pos),toInteger<std::size_t>(Str(str,order_pos)).second };
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
		} id;
		constexpr static char16_t* table[] = {
			u"ALLOC_CLASS",
			u"SUBSTITUTE",
		};
		union Data {
			std::int32_t i32[2];
			std::uint64_t u64;
		} data;
		Command(Id i, std::uint32_t d1, std::uint32_t d2) {
			id = i;
			data.i32[0] = d1;
			data.i32[1] = d2;
		}
	};

	struct FuncInfo {// This is in DoubleKeyMap.
		std::size_t param_num;
		std::size_t default_size;
		Vector<Command> command;
	};
	struct ClassInfo {// This is in DoubleKeyMap.
		std::size_t size;// The hole size for initialization. This means only member_value
		std::size_t num_of_member_param;// How many the parent FuncInstance should reserve Vector<ParamPos>.

		Vector<std::size_t> param_indexes;// Member params. Vector of DoubleKeyMap of ClassInfo.
		Vector<std::size_t> func_indexes;// Member functions. Vector of DoubleKeyMap of FuncInfo.
	};
	struct ParamPos { // This is like pointer. The memory is allocated by the parent's FuncInstance or ClassInstance.
		std::size_t info_index;// This is index of the DoubleKeyMap of ClassInfo.
		std::size_t begin_of_memory;// This is index of the member param of memory 
		std::size_t length;// All Param are treated as array.
	};
	struct FuncInstance {
		std::size_t info_index;// This is index of the DoubleKeyMap of FuncInfo.

		Vector<ParamPos> memory_use;// Vector's index is param index.
		Vector<std::uint8_t> memory;

		FuncInstance(std::size_t index, std::size_t param_num, std::size_t default_memory_size){
			info_index = index;
			memory_use.resize(param_num);
			// In constructor, memory is served but not assigned.
			// assignment is runned in runtime.
			memory.reserve(default_memory_size);
		}
	};
	struct ClassInstance {
		Vector<ParamPos> member_value;
		Vector<std::size_t> memory;

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