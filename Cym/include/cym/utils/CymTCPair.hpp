#ifndef CYM_TCPAIR_HPP
#define CYM_TCPAIR_HPP

namespace cym {
	// Suitable with memcpy()
	template<class T, class Y>
	struct Pair {
		T first;
		Y second;
		template<class T1,class Y1>
		Pair(T1 &&f, Y1 &&s) : first(std::forward<T1>(f)), second(std::forward<Y1>(s)) {

		}
	};
	template<class T,class Y>
	Pair<T,Y> makePair(const T &l, const Y &r) {
		return Pair<T, Y>(l,r);
	}
}

#endif
