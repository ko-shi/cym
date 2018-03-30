#ifndef CYM_TCPAIR_HPP
#define CYM_TCPAIR_HPP

namespace cym {
	// Suitable with memcpy()
	template<class T, class Y>
	struct Pair {
		T first;
		Y second;
		Pair(const T &f, const Y &s) : first(f), second(s) {

		}
		Pair(T &&f, Y &&s) : first(f), second(s) {

		}
	};
	template<class T,class Y>
	Pair<T,Y> makePair(T &&l, Y &&r) {
		return Pair<T, Y>(std::forward<T>(l), std::forward<Y>(r));
	}
}

#endif
