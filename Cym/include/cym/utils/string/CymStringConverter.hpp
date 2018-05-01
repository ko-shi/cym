#ifndef CYM_STRING_CONVERTER_HPP
#define CYM_STRING_CONVERTER_HPP

#include<string>
#include<memory>

#include<cym/utils/string/CymU8String.hpp>
#include<cym/utils/string/CymShiftJisString.hpp>

#if defined(_WIN32) || defined(_WIN64)
#	include<Windows.h>
#elif defined(__linux__)
#	error ""
#elif defined(__APPLE__)
#	error ""
#endif


namespace cym {
	enum CodePage {
		shift_jis_codepage = 932,
		utf_8_codepage = CP_UTF8,
		utf_16le_codepage = 1200
	};
	// length must be included '\0'
	std::wstring toWString(const char *str,int length, UINT codepage) {
		const int in_length = length;
		const int out_length = MultiByteToWideChar(codepage, 0, str, in_length, nullptr, 0);
		auto buffer = std::make_unique<wchar_t[]>(out_length + 1);
		if (out_length == 0) {
			// something error happened 
			return std::wstring();
		}
		MultiByteToWideChar(codepage, 0, str, in_length, buffer.get(), out_length);
		return std::wstring(buffer.get(), out_length);
	}
	std::wstring toWString(const std::string str, UINT codepage) {
		return toWString(str.c_str(), str.length() + 1, codepage);
	}
	std::wstring toWString(const cym::U8String &str) {
		return toWString(str.data(), str.size(), utf_8_codepage);
	}
	std::wstring toWString(const cym::SJisString &str) {
		return toWString(str.data(), str.size(), shift_jis_codepage);
	}
	std::string toCodePage(const std::wstring str, UINT codepage) {
		int in_length = static_cast<int>(str.length());
		int out_length = WideCharToMultiByte(codepage, 0, str.c_str(), in_length, 0, 0, 0, 0);
		auto buffer = std::make_unique<char[]>(out_length);
		if (out_length) {
			WideCharToMultiByte(codepage, 0, str.c_str(), in_length, buffer.get(), out_length, 0, 0);
		}
		return std::string(buffer.get(), out_length);
	}
	std::string toCodePage(const U8String &s, UINT codepage)
	{
		const std::wstring wstr = toWString(s.toString(), utf_8_codepage);
		return toCodePage(wstr, codepage);
	}

	U8String toU8String(const SJisString &str) {
		std::wstring wide = toWString(str.toString(),shift_jis_codepage);
		return U8String(toCodePage(wide,utf_8_codepage));
	}
	SJisString toSJisString(const U8String &str) {
		return SJisString(toCodePage(str, shift_jis_codepage));
	}
	/* Unicode to Unicode */
	
	U8Char toU8Char(const char32_t ch) {
		union Interpreter{
			char32_t ch32;
			char ch8[4];
		};
		if (ch < 0 || ch > 0x10FFFF) {
			return false;
		}

		if (ch < 128) {
			Interpreter temp;
			temp.ch32 = ch;
			return U8Char(&temp.ch8[0], 1);
		}
		else if (ch < 2048) {
			char temp[4] = { 0xC0 | char(ch >> 6) ,0x80 | (char(ch) & 0x3F) };
			return U8Char(&temp[0], 2);
		}
		else if (ch < 65536) {
			char temp[4] = { 0xE0 | char(ch >> 12) ,0x80 | (char(ch >> 6) & 0x3F),0x80 | (char(ch) & 0x3F) };
			return U8Char(&temp[0], 3);
		}
		else {
			char temp[4] = { 0xF0 | char(ch >> 18) ,0x80 | (char(ch >> 12) & 0x3F),0x80 | (char(ch >> 6) & 0x3F),0x80 | (char(ch) & 0x3F) };
			return U8Char(&temp[0], 4);
		}

	}

	char32_t toU32Ch(U8Char u8Ch) {
		const auto byte_size = u8Ch.size();
		if (byte_size == 0) {
			return U'\0';
		}
		switch (byte_size) {
		case 1:
			return static_cast<char32_t>(static_cast<std::uint8_t>(u8Ch[0]));
		case 2:
			return (static_cast<char32_t>(u8Ch[0] & 0x1F) << 6) | u8Ch[1] & 0x3F;
		case 3: {
			auto u32Ch = static_cast<char32_t>(u8Ch[0] & 0x0F) << 12;
			u32Ch |= static_cast<char32_t>(u8Ch[1] & 0x3F) << 6;
			u32Ch |= static_cast<char32_t>(u8Ch[2] & 0x3F);
			return u32Ch;
		}
		case 4: {
			auto u32Ch = static_cast<char32_t>(u8Ch[0] & 0x07) << 18;
			u32Ch |= static_cast<char32_t>(u8Ch[1] & 0x3F) << 12;
			u32Ch |= static_cast<char32_t>(u8Ch[2] & 0x3F) << 6;
			u32Ch |= static_cast<char32_t>(u8Ch[3] & 0x3F);
			return u32Ch;
		}
		}
		return U'\0';
	}
	char16_t toU16Ch(const char32_t u32Ch) {
		return static_cast<char16_t>(u32Ch);
	}
	std::u16string toU16String(const U8String &str) {
		std::u16string buf;
		for (auto itr = str.begin(); itr != str.end(); itr++) {
			if (toU32Ch(*itr) == u'\0') {
				break;
			}
			buf.push_back(toU16Ch(toU32Ch(*itr)));
		}
		return buf;
	}
	U8String toU8String(const char16_t *str,Size size = -1) {
		if (size == static_cast<Size>(-1)) {
			size = std::char_traits<char16_t>::length(str) + 1;
		}
		U8String buffer;
		for (Size i = 0; i < size; i++) {
			buffer.pushBack(toU8Char(str[i]));
		}
		return buffer;
	}
	U8String toU8String(const std::u16string_view &str){
		return toU8String(str.data(), str.size());
	}

	SJisString toSJisString(const std::u16string_view &str) {
		return toSJisString(toU8String(str));
	}
	std::wstring toWString(const std::u16string_view &str) {
		return toWString(toU8String(str));
	}
	std::ostream& operator<<(std::ostream &l, const std::u16string_view &r) {
		return l << toSJisString(r);
	}
	std::ostream& operator<<(std::ostream &l, const U8String &r) {
		return l << toSJisString(r.toString());
	}
	/* int to string */
	U8String toU8String(int value) {
		return toU8String(SJisString(std::to_string(value)));
	}
	SJisString toSJisString(int value) {
		return SJisString(std::to_string(value));
	}
	std::u16string toU16String(int value) {
		return toU16String(toU8String(value));
	}
}

#endif
