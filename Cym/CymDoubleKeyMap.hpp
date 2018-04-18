#ifndef CYM_DOUBLE_KEY_MAP
#define CYM_DOUBLE_KEY_MAP

#include<unordered_map>
#include<vector>

namespace cym {

	/*
	The first key is Size.
	the second key is Key:template
	
	*/
	template<class Key,class T,class Hasher = std::hash<Key>>
	struct DoubleKeyMap {
	private:
		using Size = std::size_t;
	public:
		std::vector<T> indexed;// data is here
		std::unordered_map<Key, Size,Hasher> link;// there is the link to data index fron key
		Size index = 0;
	public:
		Size emplace(const Key &key,const T &data) {
			link.emplace(key, index);
			indexed.emplace_back(data);
			return ++index;
		}
		decltype(auto) operator[](Size pos) {
			return indexed[pos];
		}
		decltype(auto) operator[](const Key &key) {
			return indexed[link[key]];
		}
		Size indexOf(const Key &key) const{
			return link.at(key);
		}
	};
}

#endif
