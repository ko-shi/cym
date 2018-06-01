#ifndef CYM_FORWARD_INDEX_MAP
#define CYM_FORWARD_INDEX_MAP

#include<unordered_map>
#include<vector>

#include<cym/CymBase.hpp>

namespace cym {

	template<class Key>
	struct FIndexMap {
	public:
		std::unordered_map<Key, Size> link;
		Size index = 0;
	public:
		Size emplace(const Key &key) {
			const auto r = link.emplace(key, index);
			if (!r.second) {
				//　キーが既にあったとき新しい値で代入
				link[key] = index;
			}
			return ++index;
		}
		Size operator[](const Key &key) const {
			return link.at(key);
		}
	};
}

#endif
