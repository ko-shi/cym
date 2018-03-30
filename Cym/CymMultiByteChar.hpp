#ifndef CYM_MULTIBYTE_CHAR_VIEW_HPP
#define CYM_MULTIBYTE_CHAR_VIEW_HPP

#include<string>
#include<cstdint>
#include<array>

#include"CymStringView.hpp"

namespace cym {
	template<std::size_t N>//MaxByte
	class MultiByteChar {
	private:
		using Traits = std::char_traits<char>;
	protected:
		std::array<char, N> arr_;
	public:
		MultiByteChar(){
			arr_.fill(0);
		}
		MultiByteChar(const MultiByteChar&) = default;
		MultiByteChar(MultiByteChar&&) = default;
		/* for literal */
		template<std::size_t N1>
		constexpr MultiByteChar(const char(&str)[N1]) : MultiByteChar(str, std::make_index_sequence<N1>()) {

		}
		template<std::size_t N,std::size_t ...I>
		constexpr MultiByteChar(const char(&str)[N], std::index_sequence<I...>) : arr_{ str[I]... } {

		}
		MultiByteChar(const char *str) {
			const auto size = Traits::length(str);
			assign(str, size);
		}
		/* for operator[] or something like that */
		MultiByteChar(const char *str, std::size_t size){
			assign(str, size);
		}
		MultiByteChar(StringView str, std::size_t size){
			assign(str.data(), size);
		}
		void* assign(const char *str, std::size_t size) {
			std::memset(arr_.data() + size, 0, N - size);
			return std::memcpy(arr_.data(), str, size);
		}
		int operator==(const MultiByteChar &r)const {
			return std::memcmp(arr_.data(),r.data(),N);
		}
		const char* data()const {
			return arr_.data();
		}
		const char operator[](std::size_t pos)const {
			return arr_[pos];
		}
		std::size_t size()const {
			std::size_t size = 0;
			for (const auto &i : arr_) {
				if(i != '\0')size++;
			}
			return size;
		}
		std::string toString()const {//add '\0'
			return std::string(arr_.data());
		}
	};
	struct U8Char : public MultiByteChar<4> {
		using MultiByteChar::MultiByteChar;
		std::size_t size()const {
			return arr_[3] == 0 ? (arr_[2] == 0 ? (arr_[1] == 0 ? (arr_[0] == 0 ? 0 : 1) : 2) : 3) : 4;
		}
	};
	struct SJisChar : MultiByteChar<2> {
		using MultiByteChar::MultiByteChar;
		std::size_t size()const {
			return arr_[1] == 0 ? (arr_[0] == 0 ? 0 : 1) : 2;
		}
		bool isThisExternalChar()const {
			return MultiByteChar::size() == 2 ?
				0xF040 <= (*static_cast<const std::uint16_t*>(static_cast<const void*>(MultiByteChar::arr_.data())))
				: false;
		}
		bool isThisPlatformDependentChar()const {
			const auto ch = (*static_cast<const std::uint16_t*>(static_cast<const void*>(MultiByteChar::arr_.data())));
			return MultiByteChar::size() == 2 ?
				0x8540 <= ch && ch <= 0x889E
				: false;
		}
		bool isThisForMac()const {
			const auto ch = (*static_cast<const std::uint16_t*>(static_cast<const void*>(MultiByteChar::arr_.data())));
			return MultiByteChar::size() == 2 ?
				0xEB40 <= ch && ch <= 0xEFFC
				: false;
		}
		bool isThisFormalChar()const {
			return !(isThisExternalChar() || isThisForMac() || isThisPlatformDependentChar());
		}

	};
}

#endif
