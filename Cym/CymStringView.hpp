#ifndef CYM_STRING_VIEW_HPP
#define CYM_STRING_VIEW_HPP

#include<string>

namespace cym {
	static char error_v = -1;
	class StringView {
	private:
		const char* ptr_;
		std::size_t size_;//included \0
	public:
		constexpr StringView() : ptr_{}, size_{} {

		}
		template<std::size_t L>
		StringView(const char(&str)[L]) : ptr_{ str }, size_{ L } {

		}
		StringView(std::string &str) : ptr_{ str.data() }, size_{ str.length() } {

		}
		StringView(std::string &str, std::size_t size) : ptr_{ str.data() }, size_{ size } {

		}
		constexpr StringView(const char *ptr, std::size_t size) : ptr_{ ptr }, size_{ size } {

		}
		const char* data() const {
			return ptr_;
		}
		constexpr std::size_t size() const {
			return size_;
		}
		constexpr std::size_t length() const {
			return size_;
		}
		constexpr char operator[](std::size_t pos) const {
			return ptr_[pos];
		}
		constexpr char at(std::size_t pos) const {
			return pos > size_ ? error_v : ptr_[pos];
		}
		constexpr StringView subStr(std::size_t begin, std::size_t len) {
			return StringView(ptr_ + begin, len);
		}
		constexpr bool operator==(const StringView &str) const {
			return size_ == str.size_ ? compare(str) : false;
		}
		bool compare(const StringView &str) const {//•¶Žš—ñ‚Ì’·‚³‚ª“¯‚¶‘O’ñ
			for (std::size_t i = 0; i < str.size(); i++) {
				if (str[i] != ptr_[i]) {
					return false;
				}
			}
			return true;
		}
		const char* begin() const {
			return ptr_;
		}
		const char* end() const {
			return ptr_ + size_;
		}
	};
}

namespace std {
	template<>
	class hash<cym::StringView> {
	public:
		size_t operator () (const cym::StringView &str) const { return hash<const char*>()(str.begin()); }
	};
}

#endif
