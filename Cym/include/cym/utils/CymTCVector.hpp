#ifndef CYM_VECTOR_HPP
#define CYM_VECTOR_HPP
#include<memory>
#include<vector>
#include<type_traits>

#include<cym/CymBase.hpp>

namespace cym {
	template<class T, class Allocator = std::allocator<T>, bool B = std::is_trivially_copyable_v<T>>
	class TCVector {
	public:
		using iterator = T*;
	private:
		using Size = std::size_t;
	private:
		Allocator allocator_;
		Size size_;
		Size capacity_;
		T* ptr_;
	public:
		TCVector() : allocator_(), size_(0), capacity_(0), ptr_(nullptr) {

		}
		TCVector(const TCVector<T> &v) : allocator_(), size_(v.size_), capacity_(0), ptr_(nullptr) {
			reserve(v.capacity_); 
			std::memcpy(ptr_, v.ptr_, sizeof(T) * v.size_);
		}
		TCVector(TCVector<T> &&v) noexcept: allocator_(), size_(v.size_), capacity_(v.capacity_), ptr_(v.ptr_) {
			v.ptr_ = nullptr;
			
		}
		TCVector(Size capacity) : allocator_(), size_(0), capacity_(0), ptr_(nullptr) {
			reserve(capacity);
		}
		TCVector(std::initializer_list<T> init) : allocator_(), size_(0), capacity_(0), ptr_(nullptr) {
			reserve(init.size());
			for (const auto &i : init) {
				pushBack(i);
			}
		}
		TCVector<T>& operator=(const TCVector<T> &v) {
			if (ptr_) {
				allocator_.deallocate(ptr_, capacity_);
				capacity_ = 0;
				size_ = 0;
			}
			reserve(v.capacity_);
			std::memcpy(ptr_, v.ptr_, sizeof(T) * v.size_);
			return *this;
		}
		TCVector<T>& operator=(TCVector<T> &&v) noexcept{
			size_ = v.size_;
			capacity_ = v.capacity_;
			ptr_ = v.ptr_;
			v.ptr_ = nullptr;
			return *this;
		}
		int pushBack(const T &val) {
			if (size_ >= capacity_) {
				reserve(size_ * 2 + 1);
			}
			std::allocator_traits<Allocator>::construct(allocator_, ptr_ + size_, val);
			return size_++, 0;
		}
		int pushBack(const TCVector<T> &val) {
			if (size_ + val.size_ > capacity_) {
				reserve((size_ + val.size_) * 2 + 1);
			}
			std::memcpy(ptr_ + size_, val.ptr_, sizeof(T) * val.size_);
			return size_ += val.size_, 0;
		}
		template<class... Args>
		int emplaceBack(Args&&... args) {
			if (size_ >= capacity_) {
				reserve(size_ * 2 + 1);
			}
			std::allocator_traits<Allocator>::construct(allocator_, ptr_ + size_, std::forward<Args>(args)...);
			return size_++, 0;
		}
		template<bool B = std::is_scalar_v<T>,std::enable_if_t<B,std::nullptr_t> = nullptr>
		int popBack() {
			return size_--, 0;
		}
		template<bool B = std::is_scalar_v<T>,std::enable_if_t<!B, std::nullptr_t> = nullptr>
		int popBack() {
			return std::allocator_traits<Allocator>::destroy(allocator_, ptr_ + size_), size_--, 0;
		}
		int erase(Size pos) {
			if (pos >= size_) {
				return 0;
			}
			std::memcpy(ptr_ + pos, ptr_ + pos + 1, sizeof(T) * (size_ - pos - 1));
			return size_--, 0;
		}
		int erase(const T * const itr) {
			const auto pos = itr - begin();
			if (pos >= static_cast<decltype(pos)>(size_)) {
				return 0;
			}
			std::memcpy(ptr_ + pos, ptr_ + pos + 1, sizeof(T) * (size_ - pos - 1));
			return size_--, 0;
		}
		int erase(const std::pair<Size/*pos*/, Size/*diff*/> &range) {
			const auto sum = range.first + range.second;
			if (sum > size_) {
				return 0;
			}
			std::memcpy(ptr_ + range.first, ptr_ + sum, sizeof(T) * (size_ - sum));
			return size_ -= range.second, 0;
		}
		void pushFront(const T &data) {
			insert(0, data);
		}
		void insert(Size pos, const T &data) {
			if (capacity_ < size_ + 1) {
				reserve(capacity_ + 1);
			}
			std::memmove(ptr_ + pos + 1, ptr_ + pos, sizeof(T) * (size_ - pos));
			ptr_[pos] = data;
			size_++;
		}
		void insert(Size pos, const TCVector<T> &data) {
			if (capacity_ < size_ + data.size()) {
				reserve(size_ + data.size());
			}
			std::memmove(ptr_ + pos + data.size(), ptr_ + pos, sizeof(T) * (size_ - pos));
			std::memcpy(ptr_ + pos, data.data(), sizeof(T) * data.size());
			size_ += data.size();
		}
		int reserve(Size capacity) {
			if (capacity_ >= capacity)return 0;
			const auto ptr = allocator_.allocate(capacity);
			if (ptr_) {
				std::memcpy(ptr, ptr_, sizeof(T) * size_);
				allocator_.deallocate(ptr_, capacity_);
			}
			return capacity_ = capacity, ptr_ = ptr,0;
		}
		int addSize(Size additional_size) {
			if (size_ + additional_size > capacity_) {
				return resize(size_ + additional_size);
			}
			return size_ += additional_size;
		}

		void reduceSize(Size offset) {
			size_ -= offset;
		}

		int resize(Size size) {
			if (capacity_ >= size) {
				std::memset(ptr_ + size_, 0, sizeof(T) * (size - size_));
				return capacity_ = size_ = size;
			}
			const auto ptr = allocator_.allocate(size);
			if (ptr_) {
				std::memcpy(ptr, ptr_, sizeof(T) * size_);
				allocator_.deallocate(ptr_, capacity_);
			}
			ptr_ = ptr;
			std::memset(ptr_ + size_, 0, size - size_);
			return capacity_ = size_ = size;
		}
		Size size()const {
			return size_;
		}
		T* begin() {
			return ptr_;
		}
		const T* begin() const {
			return ptr_;
		}
		T* end() {
			return ptr_ + size_;
		}
		const T* end() const {
			return ptr_ + size_;
		}
		T& operator[](Size pos) {
			return pos >= size_ ? back() : ptr_[pos];
		}
		T operator[](Size pos) const {
			return pos >= size_ ? back() : ptr_[pos];
		}
		T& front() {
			return ptr_[0];
		}
		T front()const {
			return ptr_[0];
		}
		T& back() {
			return ptr_[size_ - 1];
		}
		T back()const {
			return ptr_[size_ - 1];
		}
		T* data() {
			return ptr_;
		}
		const T* data() const{
			return ptr_;
		}
		bool operator==(const TCVector<T> &right) const{
			return size_ != right.size_ ? false : std::memcmp(ptr_, right.ptr_, size_) == 0;
		}
		void swap(TCVector<T> &r) noexcept {
			constexpr auto s = sizeof(TCVector<T>);
			std::uint8_t buf[s];
			std::memcpy(buf, this, s);
			std::memcpy(this, &r, s);
			std::memcpy(&r, buf, s);
		}
		bool isEmpty()const {
			return size_ == 0;
		}
		template<class Str,class Func>
		Str toString(Func &&convFunc)const {
			Str str;
			for (const auto &i : *this) {
				str += convFunc(i);
			}
			return str;
		}
		~TCVector() {
			if (ptr_) {
				allocator_.deallocate(ptr_, capacity_);
			}
		}
	};

	template<class T,class Y>
	class TCVector<T,Y,false> : public std::vector<T>{
	public:
		using Base = std::vector<T>;
	public:
		TCVector() : Base() {

		}
		TCVector(TCVector &&v) : Base(std::forward<TCVector>(v)) {

		}
		TCVector(const TCVector &v) : Base(v) {

		}
		TCVector(std::initializer_list<T> &&init) : Base(init) {

		}
		decltype(auto) operator=(const TCVector &v) {
			return Base::operator=(v);
		}
		decltype(auto) operator=(TCVector &&v) {
			return Base::operator=(v);
		}
		int pushBack(const T &val) {
			push_back(val);
			return 0;
		}
		template<class... Args>
		int emplaceBack(Args&&... args) {
			emplace_back(std::forward<Args>(args)...);
			return 0;
		}
		int popBack() {
			pop_back();
			return 0;
		}
		template<class Str, class Func>
		Str toString(Func &&convFunc)const {
			Str str;
			for (const auto &i : *this) {
				str += convFunc(i);
			}
			return str;
		}
	};
	template<class T>
	inline void swap(TCVector<T> &l, TCVector<T> &r) noexcept {
		TCVector<T> temp(std::move(l));
		r = std::move(temp);
		l = std::move(r);
	}
}
namespace std {
	template<class T>
	struct hash<cym::TCVector<T>> {
		size_t operator()(const cym::TCVector<T> &vec) const {
			return hash<T*>()(vec.data());
		}
	};
}
#endif
