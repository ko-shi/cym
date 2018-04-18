#ifndef CYM_STRING_VIEW_HPP
#define CYM_STRING_VIEW_HPP

#include<string>

namespace cym {
	static char error_v = -1;
	class StringView {
	private:
		const char* ptr_;
		Size size_;//included \0
	public:
		constexpr StringView() : ptr_{}, size_{} {

		}
		template<Size L>
		StringView(const char(&str)[L]) : ptr_{ str }, size_{ L } {

		}
		StringView(std::string &str) : ptr_{ str.data() }, size_{ str.length() } {

		}
		StringView(std::string &str, Size size) : ptr_{ str.data() }, size_{ size } {

		}
		constexpr StringView(const char *ptr, Size size) : ptr_{ ptr }, size_{ size } {

		}
		const char* data() const {
			return ptr_;
		}
		constexpr Size size() const {
			return size_;
		}
		constexpr Size length() const {
			return size_;
		}
		constexpr char operator[](Size pos) const {
			return ptr_[pos];
		}
		constexpr char at(Size pos) const {
			return pos > size_ ? error_v : ptr_[pos];
		}
		constexpr StringView subStr(Size begin, Size len) {
			return StringView(ptr_ + begin, len);
		}
		constexpr bool operator==(const StringView &str) const {
			return size_ == str.size_ ? compare(str) : false;
		}
		bool compare(const StringView &str) const {//•¶Žš—ñ‚Ì’·‚³‚ª“¯‚¶‘O’ñ
			for (Size i = 0; i < str.size(); i++) {
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
