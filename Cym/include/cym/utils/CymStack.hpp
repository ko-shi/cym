#ifndef CYM_STACK_HPP
#define CYM_STACK_HPP


#include<cym/utils/CymTCVector.hpp>

namespace cym {

	template<class T,class Container = TCVector<T>>
	class Stack{
	private:
		using Size = std::size_t;
	public:
		Stack() : c_()
		{
		}

		Stack(const Container& _Cont) : c_(_Cont)
		{
		}
		Stack(Container&& _Cont) : c_(std::move(_Cont)){

		}
		template<class... Args>
		decltype(auto) emplace(Args&&... args){
			return c_.emplaceBack(std::forward<Args>(args)...);
		}

		bool empty() const{
			return c_.isEmpty();
		}

		Size size() const{
			return c_.size();
		}

		T& top(){
			return c_.back();
		}

		const T& top() const{
			return c_.back();
		}

		void push(const T& val){
			c_.pushBack(val);
		}

		auto pop(){
			c_.popBack();
		}
		auto reserve(Size size) {
			return c_.reserve(size);
		}
		Container& get() {
			return c_;
		}
	protected:
		Container c_;	// the underlying container
	};
}

#endif