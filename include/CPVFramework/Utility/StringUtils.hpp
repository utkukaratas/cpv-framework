#pragma once
#include <cctype>
#include <vector>
#include <memory>
#include <sstream>
#include <type_traits>
#include <string_view>
#include <algorithm>
#include "./Macros.hpp"

namespace cpv {
	/** Return whether string starts with given pattern */
	static inline bool startsWith(std::string_view str, std::string_view pattern) {
		return (str.size() >= pattern.size() &&
			str.substr(0, pattern.size()) == pattern);
	}

	/** Return whether string ends with given pattern */
	static inline bool endsWith(std::string_view str, std::string_view pattern) {
		return (str.size() >= pattern.size() &&
			str.substr(str.size() - pattern.size()) == pattern);
	}

	/**
	 * Split string with specified characters.
	 * Call func(parts, count) while split.
	 * Default split characters are empty characters.
	 */
	template <class Func>
	void splitString(
		std::string_view str, const Func& func, const char* delimiter = " \t\r\n") {
		std::size_t startIndex = 0;
		std::size_t count = 0;
		while (startIndex < str.size()) {
			auto index = str.find_first_of(delimiter, startIndex);
			auto endIndex = (index == str.npos) ? str.size() : index;
			func(str.substr(startIndex, endIndex - startIndex), count);
			index = str.find_first_not_of(delimiter, endIndex);
			startIndex = (index == str.npos) ? str.size() : index;
			++count;
		}
	}

	/**
	 * Split string with specified characters.
	 * Call func(parts, count) while split.
	 * Single char version for better performance.
	 */
	template <class Func>
	void splitString(std::string_view str, const Func& func, char delimiter) {
		std::size_t startIndex = 0;
		std::size_t count = 0;
		while (startIndex < str.size()) {
			auto index = str.find_first_of(delimiter, startIndex);
			auto endIndex = (index == str.npos) ? str.size() : index;
			func(str.substr(startIndex, endIndex - startIndex), count);
			index = str.find_first_not_of(delimiter, endIndex);
			startIndex = (index == str.npos) ? str.size() : index;
			++count;
		}
	}

	/**
	 * Join arguments into string.
	 * This function is very slow, don't call it where performance is important.
	 */
	template <class Delimiter, class... Args>
	std::string joinString(Delimiter&& delimiter, Args&&... args) {
		std::ostringstream stream;
		bool isFirst = true;
		auto func = [&stream, &isFirst, &delimiter](auto&& arg) {
			if (isFirst) {
				isFirst = false;
			} else {
				stream << delimiter;
			}
			stream << arg;
			return 0;
		};
		int dummy[sizeof...(Args)] = { func(std::forward<Args>(args))... };
		return stream.str();
	}

	/**
	 * Return sub string view that removes given prefix and suffix.
	 */
	template <bool TrimPrefix = true, bool TrimSuffix = true>
	std::string_view trimString(std::string_view str, const char* removes = " \t\r\n") {
		const char* ptr = str.begin();
		const char* ptrEnd = str.end();
		std::string_view rm(removes);
		if constexpr (TrimPrefix) {
			for (;ptr < ptrEnd && rm.find_first_of(*ptr) != rm.npos; ++ptr) { }
		}
		if constexpr (TrimSuffix) {
			for (;ptr < ptrEnd && rm.find_first_of(*(ptrEnd - 1)) != rm.npos;--ptrEnd) { }
		}
		return std::string_view(ptr, ptrEnd - ptr);
	}

	/**
	 * Return sub string view that removes given prefix and suffix.
	 * Single char version for better performance.
	 */
	template <bool TrimPrefix = true, bool TrimSuffix = true>
	std::string_view trimString(std::string_view str, char removes) {
		const char* ptr = str.begin();
		const char* ptrEnd = str.end();
		if constexpr (TrimPrefix) {
			for (;ptr < ptrEnd && *ptr == removes; ++ptr) { }
		}
		if constexpr (TrimSuffix) {
			for (;ptr < ptrEnd && *(ptrEnd - 1) == removes; --ptrEnd) { }
		}
		return std::string_view(ptr, ptrEnd - ptr);
	}

	/**
	 * Convert integer to hex and write to string.
	 * IntType can be any of int??_t and uint??_t.
	 * Also see stackoverflow 5100718.
	 */
	template <class IntType, class StringType>
	void dumpIntToHex(IntType value, StringType& str) {
		static const constexpr char digits[] = "0123456789ABCDEF";
		static const constexpr std::size_t hexLen = sizeof(IntType) * 2;
		for (std::size_t i = 0, j = (hexLen - 1) * 4; i < hexLen; ++i, j -= 4) {
			str.append(1, digits[(value >> j) & 0xf]);
		}
	}

	/**
	 * Convert integer to decimal and write to string.
	 * IntType can be any of int??_t and uint??_t.
	 */
	template <class IntType, class StringType>
	void dumpIntToDec(IntType value, StringType& str) {
		static const constexpr char digits[] = "0123456789";
		if constexpr (std::numeric_limits<IntType>::is_signed) {
			if (value < 0) {
				str.append(1, '-');
			}
		}
		std::size_t prevSize = str.size();
		do {
			IntType rem = value % 10;
			value /= 10;
			str.append(1, digits[rem >= 0 ? rem : -rem]);
		} while (value != 0);
		std::reverse(str.begin() + prevSize, str.end());
	}

	/** Convert bytes to hex and write to string */
	template <class StringType>
	void dumpBytesToHex(const char* bytes, std::size_t length, StringType& str) {
		const char* start = bytes;
		const char* end = bytes + length;
		while (start < end) {
			std::uint8_t byte = static_cast<std::uint8_t>(*start++);
			dumpIntToHex(byte, str);
		}
	}

	/**
	 * Convert dynamic size hex string to integer.
	 * Return whether the coversion is successful.
	 */
	template <class IntType>
	bool loadIntFromHex(const char* hex, std::size_t hexLen, IntType& value) {
		using UnsignedIntType = std::make_unsigned_t<IntType>;
		std::uint8_t diff = 0;
		UnsignedIntType uvalue = 0; // avoid sanitizer warning: left shift of negative value
		const char* hexEnd = hex + hexLen;
		for (; hex < hexEnd; ++hex) {
			std::uint8_t c = static_cast<std::uint8_t>(*hex);
			if (c == 0) {
				return false;
			} else if ((diff = c - '0') <= 9) {
				uvalue = (uvalue << 4) | diff;
			} else if ((diff = c - 'a') <= 5) {
				uvalue = (uvalue << 4) | (diff + 0xa);
			} else if ((diff = c - 'A') <= 5) {
				uvalue = (uvalue << 4) | (diff + 0xa);
			} else {
				return false;
			}
		}
		value = static_cast<IntType>(uvalue);
		return true;
	}

	/**
	 * Convert fixed size (sizeof(IntType)*2) hex string to integer.
	 * Return whether the coversion is successful.
	 */
	template <class IntType>
	bool loadIntFromHex(const char* hex, IntType& value) {
		return loadIntFromHex<IntType>(hex, sizeof(IntType)*2, value);
	}

	/**
	 * Convert decimal string to integer.
	 * Return whether the coversion is successful.
	 * Notice convert minus decimal string to unsigned integer is an error,
	 * but overflow isn't an error (the overhead of overflow detection is high).
	 */
	template <class IntType>
	bool loadIntFromDec(const char* dec, std::size_t decLength, IntType& value) {
		const char* decEnd = dec + decLength;
		bool minus = false;
		if (CPV_UNLIKELY(dec < decEnd && *dec == '-')) {
			minus = true;
			++dec;
		}
		std::uint8_t diff = 0;
		value = 0;
		while (CPV_LIKELY(dec < decEnd)) {
			if (CPV_LIKELY((diff = *dec - '0') <= 9)) {
				value *= 10;
				value += diff;
				++dec;
			} else {
				return false;
			}
		}
		if constexpr (!std::numeric_limits<IntType>::is_signed) {
			if (minus) {
				return false;
			}
		} else if (minus) {
			value = -value;
		}
		return true;
	}

	/**
	 * Convert hex string to bytes.
	 * Return whether the coversion is successful.
	 */
	template <class StringType>
	bool loadBytesFromHex(const char* hex, std::size_t hexLength, StringType& str) {
		const char* start = hex;
		const char* end = hex + hexLength;
		char c = 0;
		while (start < end && *start != 0) {
			if (!loadIntFromHex<char>(start, c)) {
				return false;
			}
			str.append(1, c);
			start += 2;
		}
		return start == end;
	}

	/** Compare two string case insensitive */
	bool caseInsensitiveEquals(std::string_view a, std::string_view b);

	/** Get size of string contents */
	template <class StringType>
	std::size_t sizeofString(StringType&& str) {
		return std::size(str);
	}

	/** Get size of c string contents (exclude tailing zero) */
	template <std::size_t Size>
	std::size_t sizeofString(const char(&)[Size]) {
		static_assert(Size > 0, "size of c string should not be 0");
		return Size - 1;
	}

	/** Construct string with fixed size, string may contains \x00 */
	template <std::size_t Size>
	std::string makeBinaryString(const char(&str)[Size]) {
		static_assert(Size > 0, "string must be null terminated");
		return std::string(str, Size - 1);
	}
}

