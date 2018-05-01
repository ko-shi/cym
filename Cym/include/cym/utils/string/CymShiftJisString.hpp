#ifndef CYM_SHIFT_JIS_STRING_HPP
#define CYM_SHIFT_JIS_STRING_HPP

#include<memory>
#include<iterator>
#include<string>
#include<ostream>

#include<cym/utils/string/CymMultiByteChar.hpp>

namespace cym {
	struct  SJisStringTraits {
		static int getByteNumUnsafe(const char *ptr) {
			const std::uint8_t val = static_cast<std::uint8_t>(*ptr);
			return (0x81 <= val && val <= 0x9F) || (0xE0 <= val && val <= 0xEF) ? 2 : 1;
		}
		static SJisChar toSJisCharUnsafe(const char *ptr) {
			return SJisChar(ptr, getByteNumUnsafe(ptr));
		}
	};

	class SJisString;

	struct SJisCharReplacer {
		char *ptr;
		SJisString &parent;
		SJisCharReplacer(char *p, SJisString &s);
		SJisString& operator=(const SJisChar &ch);
		operator SJisChar();
	};
	class SJisStringConstIterator {
	public:
		using value_type = const SJisChar;
		using difference_type = std::ptrdiff_t;
	private:
		friend SJisString;
	private:
		using Traits = SJisStringTraits;
	private:
		const char* ptr_;
	private:
		SJisStringConstIterator helperOfOperatorPlus(Size n, const char *ptr) const {
			return n == 0 ? SJisStringConstIterator(ptr) : helperOfOperatorPlus(n - 1, ptr + Traits::getByteNumUnsafe(ptr));
		}
	public:
		SJisStringConstIterator(const char* ptr) : ptr_(ptr) {

		}
		SJisStringConstIterator() : ptr_(nullptr) {

		}
		SJisChar operator*() const {
			return Traits::toSJisCharUnsafe(ptr_);
		}
		std::unique_ptr<SJisChar> operator->()const {
			return std::make_unique<SJisChar>(Traits::toSJisCharUnsafe(ptr_));
		}
		SJisStringConstIterator& operator++() {
			return ptr_ += Traits::getByteNumUnsafe(ptr_), *this;
		}
		SJisStringConstIterator operator++(int) {
			const auto temp = ptr_;
			return ptr_ += Traits::getByteNumUnsafe(ptr_), SJisStringConstIterator(temp);
		}
		SJisStringConstIterator& operator+=(Size n) {
			return n == 0 ? *this : SJisStringConstIterator::operator++(), SJisStringConstIterator::operator+=(n - 1);
		}
		SJisStringConstIterator operator+(Size n)const {
			return helperOfOperatorPlus(n, ptr_);
		}
		bool operator==(const SJisStringConstIterator &itr)const {
			return ptr_ == itr.ptr_;
		}
		bool operator!=(const SJisStringConstIterator &itr)const {
			return ptr_ != itr.ptr_;
		}
		bool operator<(const SJisStringConstIterator &itr)const {
			return ptr_ < itr.ptr_;
		}
		bool operator>(const SJisStringConstIterator &itr)const {
			return ptr_ > itr.ptr_;
		}
		bool operator<=(const SJisStringConstIterator &itr)const {
			return ptr_ <= itr.ptr_;
		}
		bool operator>=(const SJisStringConstIterator &itr)const {
			return ptr_ >= itr.ptr_;
		}
	};

	class SJisStringIterator {
	public:
		using value_type = SJisChar;
		using difference_type = std::ptrdiff_t;
		using pointer = SJisChar*;
		using reference = SJisChar&;
	private:
		friend SJisString;
	private:
		using Traits = SJisStringTraits;
	private:
		SJisString *str_p_;
		char *ptr_;
	private:
		SJisStringIterator helperOfOperatorPlus(Size n, char *ptr) const {
			return n == 0 ? SJisStringIterator(str_p_, ptr) : helperOfOperatorPlus(n - 1, ptr + Traits::getByteNumUnsafe(ptr));
		}
	public:
		SJisStringIterator(SJisString *parent, char* ptr);
		SJisStringIterator();
		SJisCharReplacer operator*();
		std::unique_ptr<SJisChar> operator->()const;
		SJisStringIterator& operator++();
		SJisStringIterator operator++(int);
		SJisStringIterator& operator--();
		SJisStringIterator operator--(int);
		SJisStringIterator& operator+=(Size n);
		SJisStringIterator operator+(Size n)const;
		bool operator==(const SJisStringIterator &itr)const;
		bool operator!=(const SJisStringIterator &itr)const;
		bool operator<(const SJisStringIterator &itr)const;
		bool operator>(const SJisStringIterator &itr)const;
		bool operator<=(const SJisStringIterator &itr)const;
		bool operator>=(const SJisStringIterator &itr)const;
	};

	class SJisString {
	public:
		using Traits = SJisStringTraits;
		using Iterator = SJisStringIterator;
		using ConstIterator = SJisStringConstIterator;
		enum ErrorCodeGetByteNum {
			LACKOFMEMORY = 0,
			WRONGFULCHAR = -1,
			HALFWAY = -2,
		};
	private:
	private:
		friend SJisStringIterator;
		friend SJisCharReplacer;
	private:
		std::unique_ptr<char[]> ptr_;
		Size size_;//�����̓������������̃o�C�g���B\0���܂�
		Size capacity_;
	private:
		char* data() {
			return ptr_.get();
		}
	public:
		SJisString() {

		}
		SJisString(const char *str) {
			Size str_size = std::char_traits<char>::length(str) + 1;
			ptr_.reset(new char[str_size]);
			size_ = str_size;
			capacity_ = size();
			memcpy(data(), str, size());
		}
		SJisString(const std::string &str) {
			Size str_size = str.size() + 1;
			ptr_.reset(new char[str_size]);
			size_ = str_size;
			capacity_ = size();
			memcpy(data(), str.c_str(), size());
		}
		SJisString(const SJisString &sstr) : ptr_(new char[sstr.size_]), size_(sstr.size_), capacity_(size_) {
			std::memcpy(data(), sstr.data(), size());
		}
		SJisString(SJisString &&sstr) : ptr_(std::move(sstr.ptr_)), size_(sstr.size()), capacity_(size_) {

		}
		SJisString operator=(const SJisString &sstr) {
			capacity_ = size_ = sstr.size();
			ptr_.reset(new char[sstr.size()]);
			std::memcpy(data(), sstr.data(), size());
			return *this;
		}
		// non-const version is private member
		const char* data()const {
			return ptr_.get();
		}
		// getter of size_,included '\0'
		Size size() const {
			return size_;
		}
		// getter of capacity_
		Size capacity()const {
			return capacity_;
		}
		SJisString& setNull() {
			data()[size() - 1] = '\0';
			return *this;
		}
		bool isThisPointer(const char *ptr) const {
			return 0 <= ptr - data() && ptr - data() < static_cast<std::ptrdiff_t>(size());
		}
		/* This ptr must satisfy isThisPointer() == true */
		Size fowardSize(const char *ptr)const {
			return size() - (ptr - data());
		}
		bool isOnSecondByte(const char *ptr/*  It is necessary that isThisPointer() is true */) const {
			const auto first_byte = static_cast<std::uint8_t>(*ptr);
			const auto previous_byte = static_cast<std::uint8_t>(*(ptr - 1));
			return ptr - data() == 0 ?
				false
				: (0x40 <= first_byte && first_byte <= 0x7E) || (0x80 <= first_byte && first_byte <= 0xFC) ?
				(0x81 <= previous_byte && previous_byte <= 0x9F) || (0xE0 <= previous_byte && previous_byte <= 0xEF)
				: false;
		}
		/* more safe than Traits's one */
		int getByteNum(const char *ptr) const {
			return isOnSecondByte(ptr) ? HALFWAY : Traits::getByteNumUnsafe(ptr);
		}
		/* more safe than Traits's one */
		SJisChar toSJisChar(const char *ptr) const {
			if (const auto r = getByteNum(ptr); r <= 0) {
				return SJisChar();
			}
			else {
				return Traits::toSJisCharUnsafe(ptr);
			}
		}
		Iterator decreaseIterator(const Iterator &itr) {
			const auto forward_size = fowardSize(itr.ptr_);
			if (forward_size >= 1) {
				if (forward_size == 1) {
					return Iterator(this,itr.ptr_ - 1);
				}
				else {
					return isOnSecondByte(itr.ptr_ - 1) ? Iterator(this,itr.ptr_ - 2) : Iterator(this,itr.ptr_ - 1);
				}
			}
			return Iterator();
		}
		SJisString& reserve(Size s) {
			if (s > capacity_) {
				capacity_ = s;
				auto temp = std::make_unique<char[]>(capacity_);
				std::memcpy(temp.get(), data(), size());
				ptr_ = std::move(temp);
			}
			return *this;
		}
		SJisString& pushBack(const SJisChar &ch) {
			if (capacity_ < size() + ch.size()) {
				reserve(static_cast<Size>((size() + 2) * 1.5));
			}
			std::memcpy(data() + size() - 1, ch.data(), ch.size());
			size_ += ch.size();
			setNull();
			return *this;
		}
		SJisString operator+(const char *str) {
			SJisString temp(*this);
			const auto str_size = std::char_traits<char>::length(str);
			temp.reserve(temp.size() + str_size - 1);//-1 for '\0'
			std::memcpy(temp.data() + temp.size() - 2, str, str_size);
			temp.size_ += str_size - 1;
			setNull();
			return temp;
		}
		SJisString operator+(const SJisString &str) {
			SJisString temp(*this);
			temp.reserve(temp.size() + str.size() - 1);//-1 for '\0'
			std::memcpy(temp.data() + temp.size() - 2, str.data(), str.size());
			temp.size_ += str.size() - 1;
			temp.setNull();
			return temp;
		}
		SJisString operator+=(const char *str) {
			const auto str_size = std::char_traits<char>::length(str);
			reserve(size() + str_size - 1);//-1 for '\0'
			std::memcpy(data() + size() - 2, str, str_size);
			size_ += str_size - 1;
			setNull();
			return *this;
		}
		SJisString operator+=(const SJisString &str) {
			reserve(size() + str.size() - 1);//-1 for '\0'
			std::memcpy(data() + size() - 2, str.data(), str.size());
			size_ += str.size() - 1;
			setNull();
			return *this;
		}
		SJisChar operator[](Size n)const {
			ConstIterator itr(data());
			for (Size i = 0; i < n; i++) {
				itr++;
			}
			return *itr;
		}
		SJisCharReplacer operator[](Size n) {
			Iterator itr(this,data());
			for (Size i = 0; i < n; i++) {
				itr++;
			}
			return SJisCharReplacer(itr.ptr_, *this);
		}
		SJisChar at(Size n)const {
			ConstIterator itr(data());
			for (Size i = 0; i < n; i++) {
				if (end() == itr++) {
					return SJisChar();
				}
			}
			return *itr;
		}
		Size length() const {
			Size s = 0;
			for (ConstIterator itr = begin(); itr != end(); itr++) {
				s++;
			}
			return s;
		}
		bool isFormalString()const {
			for (const auto &i : *this) {
				if (!i.isThisFormalChar()) {
					return false;
				}
			}
			return true;
		}
		SJisStringIterator find(const SJisChar &ch) {
			for (auto itr = begin(); itr != end(); itr++) {
				if (ch == *itr) {
					return itr;
				}
			}
			return end();
		}
		template<Size N>
		SJisStringIterator find(const char(&str)[N]) {
			for (auto itr = begin(); itr != end(); itr++) {
				if (std::memcmp(itr.ptr_, str, N - 1) == 0) {// -1 for '\0'
					return itr;
				}
			}
			return end();
		}
		SJisStringIterator find(const char *str) {
			const auto size = std::char_traits<char>::length(str);
			for (auto itr = begin(); itr != end(); itr++) {
				if (std::memcmp(itr.ptr_, str, size - 1) == 0) {// -1 for '\0'
					return itr;
				}
			}
			return end();
		}
		SJisStringIterator find(const SJisString &str) {
			for (auto itr = begin(); itr != end(); itr++) {
				if (std::memcmp(itr.ptr_, str.data(), str.size() - 1) == 0) {// -1 for '\0'
					return itr;
				}
			}
			return end();
		}
		Iterator find(const std::string_view &str) {
			for (auto itr = begin(); itr != end(); itr++) {
				if (std::memcmp(itr.ptr_, str.data(), str.size() - 1) == 0) {// -1 for '\0'
					return itr;
				}
			}
			return end();
		}
		std::string toString()const {
			return std::string(data(),size() - 1);
		}
		SJisChar front()const {
			return toSJisChar(data());
		}
		SJisChar back()const {
			const auto last_byte = data() + size() - 2;// -1 for '\0'
			return isOnSecondByte(last_byte) ? toSJisChar(last_byte - 1) : toSJisChar(last_byte);
		}
		ConstIterator begin()const {
			return ConstIterator(data());
		}
		Iterator begin() {
			return Iterator(this,data());
		}
		ConstIterator end()const {
			return ConstIterator(data() + size() - 1);// -1 for '\0'
		}
		Iterator end() {
			return Iterator(this,data() + size() - 1);// -1 for '\0'
		}
	};

	/*
	
	

						D E F I N E T I O N
	
	
	*/

	SJisCharReplacer::SJisCharReplacer(char *p, SJisString &s) : ptr(p), parent(s) {

	}
	SJisString& SJisCharReplacer::operator=(const SJisChar &ch) {
		const auto old_size = static_cast<Size>(parent.getByteNum(ptr));
		if (old_size == ch.size()) {
			std::memcpy(ptr, ch.data(), old_size);
		}
		else if (old_size == 1) {

			parent.reserve(parent.size_++);
			std::memmove(ptr + 1, ptr, (parent.data() + parent.size()) - ptr);
			std::memcpy(ptr, ch.data(), 2);
		}
		else if (old_size == 2) {
			parent.size_--;
			std::memmove(ptr, ptr + 1, (parent.data() + parent.size()) - ptr);
			*ptr = *ch.data();
		}
		return parent;
	}
	SJisCharReplacer::operator SJisChar() {
		return parent.toSJisChar(ptr);
	}




	SJisStringIterator::SJisStringIterator(SJisString *parent, char* ptr) : str_p_(parent), ptr_(ptr) {

	}
	SJisStringIterator::SJisStringIterator() : str_p_(nullptr), ptr_(nullptr) {

	}
	SJisCharReplacer SJisStringIterator::operator*() {
		return SJisCharReplacer(ptr_, *str_p_);
	}
	std::unique_ptr<SJisChar> SJisStringIterator::operator->()const {
		return std::make_unique<SJisChar>(Traits::toSJisCharUnsafe(ptr_));
	}
	SJisStringIterator& SJisStringIterator::operator++() {
		return ptr_ += str_p_->getByteNum(ptr_), *this;
	}
	SJisStringIterator SJisStringIterator::operator++(int) {
		const auto temp = ptr_;
		return ptr_ += str_p_->getByteNum(ptr_), SJisStringIterator(str_p_, temp);
	}
	SJisStringIterator& SJisStringIterator::operator--() {
		const auto forward_size = str_p_->fowardSize(ptr_);
		if (forward_size >= 1) {
			if (forward_size == 1) {
				ptr_--;
			}
			else {
				ptr_ -= (str_p_->isOnSecondByte(ptr_ - 1) ? 2 : 1);
			}
		}
		return *this;
	}
	SJisStringIterator SJisStringIterator::operator--(int) {
		const auto old_ptr = ptr_;
		const auto forward_size = str_p_->fowardSize(ptr_);
		if (forward_size >= 1) {
			if (forward_size == 1) {
				ptr_--;
			}
			else {
				ptr_ -= (str_p_->isOnSecondByte(ptr_ - 1) ? 2 : 1);
			}
		}
		return SJisStringIterator(str_p_, old_ptr);
	}
	SJisStringIterator& SJisStringIterator::operator+=(Size n) {
		for (Size i = 0; i < n; i++) {
			SJisStringIterator::operator++();
		}
		return *this;
	}
	SJisStringIterator SJisStringIterator::operator+(Size n)const {
		return helperOfOperatorPlus(n, ptr_);
	}
	bool SJisStringIterator::operator==(const SJisStringIterator &itr)const {
		return ptr_ == itr.ptr_;
	}
	bool SJisStringIterator::operator!=(const SJisStringIterator &itr)const {
		return ptr_ != itr.ptr_;
	}
	bool SJisStringIterator::operator<(const SJisStringIterator &itr)const {
		return ptr_ < itr.ptr_;
	}
	bool SJisStringIterator::operator>(const SJisStringIterator &itr)const {
		return ptr_ > itr.ptr_;
	}
	bool SJisStringIterator::operator<=(const SJisStringIterator &itr)const {
		return ptr_ <= itr.ptr_;
	}
	bool SJisStringIterator::operator>=(const SJisStringIterator &itr)const {
		return ptr_ >= itr.ptr_;
	}

	std::ostream& operator<<(std::ostream &l, const SJisString &r) {
		return l << r.toString();
	}
}

namespace std {
	template<>
	class hash<cym::SJisString> {
	public:
		size_t operator () (const cym::SJisString &str) const { return hash<const char*>()(str.data()); }
	};
}
#endif
