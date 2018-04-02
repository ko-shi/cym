#ifndef CYM_DOUBLE_KEY_MAP
#define CYM_DOUBLE_KEY_MAP

#include<unordered_map>
#include<vector>

namespace cym {

	/*
	The first key is std::size_t.
	the second key is Key:template
	
	*/
	template<class Key,class T,class Hasher = std::hash<Key>>
	struct DoubleKeyMap {
	public:
		std::vector<T> indexed;// data is here
		std::unordered_map<Key, std::size_t,Hasher> link;// there is the link to data index fron key
		std::size_t index = 0;
	public:
		std::size_t emplace(const Key &key,const T &data) {
			link.emplace(key, index);
			indexed.emplace_back(data);
			return ++index;
		}
		decltype(auto) operator[](std::size_t pos) {
			return indexed[pos];
		}
		decltype(auto) operator[](const Key &key) {
			return indexed[link[key]];
		}
		std::size_t indexOf(const Key &key) const{
			return link.at(key);
		}
	};
}

#endif
