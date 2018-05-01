#ifndef CYM_U8STRING_HPP
#define CYM_U8STRING_HPP

#include<memory>
#include<string>
#include<array>
#include<bitset>
#include<variant>

#include<cym/utils/string/CymMultiByteChar.hpp>

namespace cym {
	struct U8StringTraits {
		static int getByteNumUnsafe(const char *ptr) {
			const auto head = static_cast<std::uint8_t>(ptr[0]);
			if ((head >> 7) == 0) {
				return 1;
			}
			else if ((head >> 5) == 0b110) {
				return 2;
			}
			else if ((head >> 4) == 0b1110) {
				return 3;
			}
			else if ((head >> 3) == 0b11110) {
				return 4;
			}
			return 0;
		}
		static U8Char toU8CharUnsafe(const char *ptr) {
			return U8Char(ptr, getByteNumUnsafe(ptr));
		}
	};
	class U8String;

	struct U8CharReplacer {
		char *ptr;
		U8String &parent;
		U8CharReplacer(char *p, U8String &s);
		U8CharReplacer(U8CharReplacer &&r);
		U8CharReplacer(const U8CharReplacer &r);
		U8String& operator=(const U8Char &ch);
		operator U8Char();
	};
	class U8StringConstIterator {
	public:
		using value_type = const U8Char;
		using difference_type = std::ptrdiff_t;
	private:
		friend U8String;
	private:
		using Traits = U8StringTraits;
	private:
		const char* ptr_;
	public:
		U8StringConstIterator(const U8StringConstIterator &itr) : ptr_(itr.ptr_) {

		}
		U8StringConstIterator(U8StringConstIterator &&itr) : ptr_(itr.ptr_) {

		}
		U8StringConstIterator() : ptr_(nullptr) {

		}
		U8StringConstIterator(const char* ptr) : ptr_(ptr) {

		}
		U8Char operator*() const {
			return Traits::toU8CharUnsafe(ptr_);
		}
		std::unique_ptr<U8Char> operator->()const {
			return std::make_unique<U8Char>(Traits::toU8CharUnsafe(ptr_));
		}
		U8StringConstIterator& operator++() {
			return ptr_ += Traits::getByteNumUnsafe(ptr_), *this;
		}
		U8StringConstIterator operator++(int) {
			const auto temp = ptr_;
			return ptr_ += Traits::getByteNumUnsafe(ptr_), U8StringConstIterator(temp);
		}
		U8StringConstIterator& operator+=(Size n) {
			for (Size i = 0; i < n; i++) {
				U8StringConstIterator::operator++();
			}
			return *this;
		}
		U8StringConstIterator operator+(Size n)const {
			U8StringConstIterator itr(ptr_);
			for (Size i = 0; i < n; i++) {
				itr++;
			}
			return itr;
		}
		bool operator==(const U8StringConstIterator &itr)const {
			return ptr_ == itr.ptr_;
		}
		bool operator!=(const U8StringConstIterator &itr)const {
			return ptr_ != itr.ptr_;
		}
		bool operator<(const U8StringConstIterator &itr)const {
			return ptr_ < itr.ptr_;
		}
		bool operator>(const U8StringConstIterator &itr)const {
			return ptr_ > itr.ptr_;
		}
		bool operator<=(const U8StringConstIterator &itr)const {
			return ptr_ <= itr.ptr_;
		}
		bool operator>=(const U8StringConstIterator &itr)const {
			return ptr_ >= itr.ptr_;
		}
	};
	class U8StringIterator {
	public:
		using value_type = U8Char;
		using difference_type = std::ptrdiff_t;
		using pointer = U8Char*;
		using reference = U8Char&;
	private:
		friend U8String;
	private:
		using Traits = U8StringTraits;
	private:
		U8String *str_p_;
		char *ptr_;
	public:
		U8StringIterator(U8String *parent, char* ptr);
		U8StringIterator();
		U8StringIterator(U8StringIterator &&itr);
		U8StringIterator(const U8StringIterator & itr);
		U8CharReplacer operator*();
		std::unique_ptr<U8Char> operator->()const;
		U8StringIterator& operator++();
		U8StringIterator operator++(int);
		U8StringIterator& operator--();
		U8StringIterator operator--(int);
		U8StringIterator& operator+=(Size n);
		U8StringIterator operator+(Size n)const;
		bool operator==(const U8StringIterator &itr)const;
		bool operator!=(const U8StringIterator &itr)const;
		bool operator<(const U8StringIterator &itr)const;
		bool operator>(const U8StringIterator &itr)const;
		bool operator<=(const U8StringIterator &itr)const;
		bool operator>=(const U8StringIterator &itr)const;
	};

	class U8String {
	public:
		using Char = std::array<char, 4>;
		enum ErrorCodeGetByteNum {
			LACKOFMEMORY = 0,
			WRONGFULCHAR = -1,
			HALFWAY = -2,
		};
	private:
		using Traits = std::char_traits<char>;
		using Iterator = U8StringIterator;
		using ConstIterator = U8StringConstIterator;
	private:
		friend U8CharReplacer;
	private:
		std::unique_ptr<char[]> ptr_;
		Size size_;//文字の入ったメモリのバイト数。\0を含む
		Size capacity_;
	private:
	public:
		template<Size N>
		U8String(const char(&str)[N]) : ptr_(new char[N]), size_(N), capacity_(size_) {
			std::memcpy(data(), str, N * sizeof(char));
		}
		U8String() : U8String(u8""){
		}
		U8String(const std::string &str) : ptr_(new char[str.length() + 1]), size_(str.length() + 1), capacity_(size_) {
			std::memcpy(data(), str.c_str(), size_ * sizeof(char));
		}
		U8String(const U8String &sstr) : ptr_(new char[sstr.size()]), size_(sstr.size()), capacity_(size_) {
			std::memcpy(data(), sstr.data(), size());
		}
		U8String(U8String &&sstr) : ptr_(std::move(sstr.ptr_)), size_(sstr.size()), capacity_(size_) {

		}
		char* data() {
			return ptr_.get();
		}
		const char* data()const {
			return ptr_.get();
		}
		Size size()const{
			return size_;
		}
		Size capacity()const {
			return capacity_;
		}
		U8String& setNull() {
			data()[size() - 1] = '\0';
			return *this;
		}
		U8String& reserve(Size s) {
			if (s > capacity()) {
				capacity_ = s;
				auto temp = new char[capacity()];
				std::memcpy(temp, data(), size());
				ptr_.reset(temp);
			}
			return *this;
		}
		bool isThisPointer(const char *ptr) const {
			return 0 <= ptr - data() && ptr - data() < static_cast<std::ptrdiff_t>(size());
		}
		int lengthToLeadByte(const char *ptr) {
			int i = 0;
			while(static_cast<unsigned char>(ptr[1]) >> 6 == 0b10) {
				i++;
			}
			return i;
		}
		/* This ptr must satisfy isThisPointer() == true */
		Size forwardSize(const char *ptr)const {
			return size() - (ptr - data());
		}
		/* This ptr must satisfy isThisPointer() == true */
		/* If this function returned the num less than 1,It means error.*/
		int getByteNum(const char *ptr)const {
			const auto head = static_cast<std::uint8_t>(ptr[0]);
			if ((head >> 7) == 0) {
				return 1;
			}
			else if ((head >> 5) == 0b110) {
				if (forwardSize(ptr) < 2) {
					return LACKOFMEMORY;
				}
				if ((head & 0b00011110) == 0) {
					return WRONGFULCHAR;
				}
				return 2;
			}
			else if ((head >> 4) == 0b1110) {
				if (forwardSize(ptr) < 3) {
					return LACKOFMEMORY;
				}
				if ((head & 0b00001111) == 0 && (static_cast<unsigned char>(ptr[1]) & 0b00100000) == 0) {
					return WRONGFULCHAR;
				}
				return 3;
			}
			else if ((head >> 3) == 0b11110) {
				if (forwardSize(ptr) < 4) {
					return LACKOFMEMORY;
				}
				if ((head & 0b00000111) == 0 && (static_cast<unsigned char>(ptr[1]) & 0b00110000) == 0) {
					return WRONGFULCHAR;
				}
				return 4;
			}
			else if (forwardSize(ptr) >= 2 && static_cast<unsigned char>(ptr[1]) >> 6 == 0b10) {
				return HALFWAY;
			}
			return WRONGFULCHAR;
		}
		/* This ptr must satisfy isThisPointer() == true */
		U8Char toU8Char(const char *ptr) const {
			if (const auto r = getByteNum(ptr); r <= 0) {
				return U8Char();
			}
			else {
				return U8Char(ptr, r);
			}
		}
		U8String& pushBack(const U8Char &ch) {
			if (capacity_ < size() + ch.size()) {
				reserve(static_cast<Size>((size() + 2) * 1.5));
			}
			std::memcpy(data() + size() - 1, ch.data(), ch.size());
			size_ += ch.size();
			setNull();
			return *this;
		}
		U8String operator+(const char *str) {
			U8String temp(*this);
			const auto str_size = std::char_traits<char>::length(str);
			temp.reserve(temp.size() + str_size - 1);//-1 for '\0'
			std::memcpy(temp.data() + temp.size() - 2, str, str_size);
			temp.size_ += str_size - 1;
			setNull();
			return temp;
		}
		U8String operator+(const U8String &str) {
			U8String temp(*this);
			temp.reserve(temp.size() + str.size() - 1);//-1 for '\0'
			std::memcpy(temp.data() + temp.size() - 2, str.data(), str.size());
			temp.size_ += str.size() - 1;
			temp.setNull();
			return temp;
		}
		U8String operator+=(const char *str) {
			const auto str_size = std::char_traits<char>::length(str);
			reserve(size() + str_size - 1);//-1 for '\0'
			std::memcpy(data() + size() - 2, str, str_size);
			size_ += str_size - 1;
			setNull();
			return *this;
		}
		U8String operator+=(const U8String &str) {
			reserve(size() + str.size() - 1);//-1 for '\0'
			std::memcpy(data() + size() - 2, str.data(), str.size());
			size_ += str.size() - 1;
			setNull();
			return *this;
		}
		U8Char operator[](Size n)const {
			ConstIterator itr(data());
			for (Size i = 0; i < n; i++) {
				itr++;
			}
			return *itr;
		}
		U8CharReplacer operator[](Size n) {
			Iterator itr(this, data());
			for (Size i = 0; i < n; i++) {
				itr++;
			}
			return U8CharReplacer(itr.ptr_, *this);
		}
		U8Char at(Size n)const {
			ConstIterator itr(data());
			for (Size i = 0; i < n; i++) {
				if (itr++ == end()) {
					return U8Char();
				}
			}
			return *itr;
		}
		Size length()const {
			Size s = 0;
			for (ConstIterator itr = begin(); itr != end(); itr++) {
				s++;
			}
			return s;
		}
		U8Char front()const {
			return toU8Char(data());
		}
		ConstIterator begin()const {
			return ConstIterator(data());
		}
		Iterator begin() {
			return Iterator(this, data());
		}
		ConstIterator end()const {
			return ConstIterator(data() + size() - 1);// -1 for '\0'
		}
		Iterator end() {
			return Iterator(this, data() + size() - 1);// -1 for '\0'
		}
		std::string toString()const {
			return std::string(data(), size() - 1);
		}
	};


	/*
	
	
				D E F N I T I O N

	
	*/



	U8CharReplacer::U8CharReplacer(char *p, U8String &s) : ptr(p), parent(s) {

	}
	U8CharReplacer::U8CharReplacer(U8CharReplacer && r) : ptr(r.ptr),parent(r.parent){

	}
	U8CharReplacer::U8CharReplacer(const U8CharReplacer & r) : ptr(r.ptr), parent(r.parent) {
	
	}
	U8String& U8CharReplacer::operator=(const U8Char &ch) {
		const auto size_dif = static_cast<int>(ch.size()) - parent.getByteNum(ptr);
		if (size_dif == 0) {
			std::memcpy(ptr, ch.data(), ch.size());
		}
		else if (size_dif > 0) {
			/* ptr_dif is for reserve(),because parents.data() will change after reserve() */
			const auto ptr_dif = ptr - parent.data();
			parent.reserve(parent.size_ += size_dif);
			ptr = parent.data() + ptr_dif;
			std::memmove(ptr + size_dif, ptr, (parent.data() + parent.size()) - ptr);
			std::memcpy(ptr, ch.data(), ch.size());
		}
		else {
			parent.size_ += size_dif;
			std::memmove(ptr, ptr - size_dif, (parent.data() + parent.size()) - ptr);
			std::memcpy(ptr, ch.data(), ch.size());
		}
		parent.setNull();
		return parent;
	}
	U8CharReplacer::operator U8Char() {
		return parent.toU8Char(ptr);
	}


	U8StringIterator::U8StringIterator(U8String *parent, char* ptr) : str_p_(parent), ptr_(ptr) {

	}
	U8StringIterator::U8StringIterator() : str_p_(nullptr), ptr_(nullptr) {

	}
	U8StringIterator::U8StringIterator(U8StringIterator && itr) : str_p_(itr.str_p_), ptr_(itr.ptr_) {

	}
	U8StringIterator::U8StringIterator(const U8StringIterator & itr) : str_p_(itr.str_p_), ptr_(itr.ptr_) {

	}
	U8CharReplacer U8StringIterator::operator*() {
		return U8CharReplacer(ptr_, *str_p_);
	}
	std::unique_ptr<U8Char> U8StringIterator::operator->()const {
		return std::make_unique<U8Char>(Traits::toU8CharUnsafe(ptr_));
	}
	U8StringIterator& U8StringIterator::operator++() {
		return ptr_ += str_p_->getByteNum(ptr_), *this;
	}
	U8StringIterator U8StringIterator::operator++(int) {
		auto temp = ptr_;
		return ptr_ += str_p_->getByteNum(ptr_), U8StringIterator(str_p_, temp);
	}
	U8StringIterator& U8StringIterator::operator--() {
		const auto forward_size = str_p_->forwardSize(ptr_);
		if (forward_size >= 1) {
			if (forward_size == 1) {
				ptr_--;
			}
			else {
				ptr_ -= str_p_->lengthToLeadByte(ptr_ - 1) + 1;
			}
		}
		return *this;
	}
	U8StringIterator U8StringIterator::operator--(int) {
		const auto old_ptr = ptr_;
		const auto forward_size = str_p_->forwardSize(ptr_);
		if (forward_size >= 1) {
			if (forward_size == 1) {
				ptr_--;
			}
			else {
				ptr_ -= str_p_->lengthToLeadByte(ptr_ - 1) + 1;
			}
		}
		return U8StringIterator(str_p_, old_ptr);
	}
	U8StringIterator& U8StringIterator::operator+=(Size n) {
		for (Size i = 0; i < n; i++) {
			U8StringIterator::operator++();
		}
		return *this;
	}
	U8StringIterator U8StringIterator::operator+(Size n)const {
		U8StringIterator itr(str_p_,ptr_);
		for (Size i = 0; i < n; i++) {
			itr++;
		}
		return itr;
	}
	bool U8StringIterator::operator==(const U8StringIterator &itr)const {
		return ptr_ == itr.ptr_;
	}
	bool U8StringIterator::operator!=(const U8StringIterator &itr)const {
		return ptr_ != itr.ptr_;
	}
	bool U8StringIterator::operator<(const U8StringIterator &itr)const {
		return ptr_ < itr.ptr_;
	}
	bool U8StringIterator::operator>(const U8StringIterator &itr)const {
		return ptr_ > itr.ptr_;
	}
	bool U8StringIterator::operator<=(const U8StringIterator &itr)const {
		return ptr_ <= itr.ptr_;
	}
	bool U8StringIterator::operator>=(const U8StringIterator &itr)const {
		return ptr_ >= itr.ptr_;
	}
}

namespace std {
	template<>
	class hash<cym::U8String> {
	public:
		size_t operator () (const cym::U8String &str) const { return hash<const char*>()(str.data()); }
	};
}
#endif 
