#ifndef CYM_TREE_HPP
#define CYM_TREE_HPP

#include<utility>
#include<variant>

#include"CymDataTypes.hpp"
#include"CymStringOperation.hpp"
#include"CymVector.hpp"

namespace cym {
	enum class TreeTypes {
		STRING,
		NUMBER,
		DECIMAL,
		BOOLEAN,
		NULL_,
		OBJECT,
		ARRAY
	};
	class Tree {
	public:
		using ObjectType = Map<Str, std::unique_ptr<Tree>>;
		using ArrayType = std::vector<std::unique_ptr<Tree>>;
		using AllTypeVariant =
			std::variant <
			Str,
			Int,
			double,
			bool,
			std::nullptr_t,
			ObjectType,
			ArrayType
			>;
	private:
		AllTypeVariant data_;

	public:
		Tree() {
			data_ = nullptr;
		}
		Tree(Tree &&tree) : data_(std::move(tree.data_)){

		}
		Tree(ObjectType &&data) : data_(std::move(data)){

		}
		Tree(ArrayType &&data) : data_(std::move(data)){
		
		}
		template<class T,bool B = (!std::is_same_v<T,ObjectType>) && (!std::is_same_v<T, ObjectType>) ,std::enable_if_t<B,std::nullptr_t> = nullptr>
		Tree(const T &data) {
			data_ = data;
		}
	private:
	public:
		template<class T>
		T& get() {
			return std::get<T>(data_);
		}
	public:
		template<class T>
		const T& get() const {
			return std::get<T>(data_);
		}
		TreeTypes getWhichHas()const {
			return static_cast<TreeTypes>(data_.index());
		}
		
		template<class T>
		void addWhenArray(T &&object) {
			if (getWhichHas() == TreeTypes::ARRAY) {
				get<ArrayType>().emplace_back(std::make_unique<T>(std::move(object)));
			}
		}
		template<class T>
		void addWhenObject(const Str &name, T &&object) {
			if (getWhichHas() == TreeTypes::OBJECT) {
				get<ObjectType>().emplace(name, std::make_unique<T>(std::move(object)));
			}
		}
		
		Str getJSON(Size indent_num = 1) const{
			const auto single_indent = u"    ";
			const Str indent = repeat(single_indent, indent_num);
			Stream str;
			switch (getWhichHas()) {
			case TreeTypes::STRING:
				str << '"' << get<Str>() << '"';
				break;
			case TreeTypes::NUMBER:
				str << get<Int>();
				break;
			case TreeTypes::DECIMAL:
				str << get<double>();
				break;
			case TreeTypes::BOOLEAN:
				str << std::boolalpha << get<bool>();
				break;
			case TreeTypes::NULL_:
				str << u"null";
				break;
			case TreeTypes::OBJECT: {
				str << u"{";
				Str temp;
				for (const auto &i : get<ObjectType>()) {
					temp += Str(u"\n") + indent + u"\"" + i.first + u"\"" + u" : " + i.second->getJSON(indent_num + 1) + u",";
				}
				if (!temp.empty()) {
					temp.pop_back();
				}
				str << temp;
				str << u"\n" << repeat(single_indent,indent_num - 1) << u"}";
			}
				break;
			case TreeTypes::ARRAY:
				str << u"[";
				Str temp;
				for (const auto &i : get<ArrayType>()) {
					temp += Str(u"\n") + indent + i.get()->getJSON(indent_num + 1) + u",";
				}
				if (!temp.empty()) {
					temp.pop_back();
				}
				str << temp;
				str << u"\n" << repeat(single_indent, indent_num - 1) << u"]";
				break;
			}
			return str.str();
		}
		
	};
}


#endif
