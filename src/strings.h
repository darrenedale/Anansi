/*
 * Copyright 2015 - 2018 Darren Edale
 *
 * This file is part of the Equit library.
 *
 * The Equit library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Equit library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Equit library. If not, see <http://www.gnu.org/licenses/>.
 */

/// \file strings.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Useful string (template) functions.
///
/// \dep
/// - <iterator>
/// - <algorithm>
/// - <type_traits>
/// - <regex>
/// - <cctype>
///
/// \par Changes
/// - (2018-03) First release.

#ifndef EQ_STRINGS_H
#define EQ_STRINGS_H

#include <iterator>
#include <algorithm>
#include <type_traits>
#include <regex>
#include <cctype>


namespace Equit {


	namespace Detail {
		// this factor is just an estimate ATM
		static constexpr const float EscapeBufferSizeFactor = 1.1f;
	}  // namespace Detail


	using std::begin;
	using std::end;
	using std::cbegin;
	using std::cend;
	using std::size;


	// equivalent of std::size for null-terminated [const] char * *MUST* be null
	// terminated
	template<typename CharArrayType>
	std::enable_if_t<std::is_same<std::decay_t<CharArrayType>, char *>::value || std::is_same<std::decay_t<CharArrayType>, const char *>::value, size_t>
	size(CharArrayType str) noexcept {
		if(!str) {
			return 0;
		}

		size_t ret = 0;

		while(*str) {
			++str;
			++ret;
		}

		return ret;
	}


	class char_iterator : public std::iterator<std::forward_iterator_tag, const char> {
	public:
		explicit char_iterator(const char * str)
		: m_current(str),
		  m_end(str + size(str)) {
		}

		char_iterator & operator++() {
			if(m_current != m_end) {
				++m_current;
			}

			return *this;
		}

		char_iterator operator++(int) {
			char_iterator ret = *this;
			++(*this);
			return ret;
		}

		bool operator==(const char_iterator & other) {
			return m_current == other.m_current;
		}

		bool operator!=(const char_iterator & other) {
			return !(*this == other);
		}

		reference operator*() const {
			return *m_current;
		}

	private:
		const char * m_current;
		const char * m_end;
	};


	// equivalent of std::begin for null-terminated char * *MUST* be null
	// terminated
	inline char_iterator begin(char * str) noexcept {
		return char_iterator(str);
	}


	// equivalent of std::end for null-terminated char * *MUST* be null terminated
	inline char_iterator end(char * str) noexcept {
		return char_iterator(str + size(str));
	}


	// equivalent of std::cbegin for null-terminated [const] char *
	// *MUST* be null terminated
	inline char_iterator cbegin(const char * str) noexcept {
		return char_iterator(str);
	}

	inline char_iterator cbegin(char * str) noexcept {
		return char_iterator(str);
	}


	// equivalent of std::cend for null-terminated [const] char *
	// *MUST* be null terminated
	inline char_iterator cend(const char * str) noexcept {
		return char_iterator(str + size(str));
	}


	inline char_iterator cend(char * str) noexcept {
		return char_iterator(str + size(str));
	}


	template<typename StringType>
	StringType to_lower(const StringType & str) {
		StringType ret;

		// use of member .cbegin() and .cend() ensures this can't work for raw [const] char *
		// which is as it should be
		std::transform(str.cbegin(), str.cend(), std::back_inserter(ret), [](const auto & ch) {
			return std::tolower(ch);
		});

		return ret;
	};


	template<typename StringType>
	StringType to_upper(const StringType & str) {
		StringType ret;

		// use of member .cbegin() and .cend() ensures this can't work for raw [const] char *
		// which is as it should be
		std::transform(str.cbegin(), str.cend(), std::back_inserter(ret), [](const auto & ch) {
			return std::toupper(ch);
		});

		return ret;
	};


	template<typename StringType, typename FragmentStringType = StringType>
	bool starts_with(const StringType & str, const FragmentStringType & fragment) {
		if(size(fragment) > size(str)) {
			return false;
		}

		auto strIt = cbegin(str);
		auto fragmentIt = cbegin(fragment);
		const auto end = cend(fragment);

		while(fragmentIt != end) {
			if(*strIt != *fragmentIt) {
				return false;
			}

			++strIt;
			++fragmentIt;
		}

		return true;
	}


	template<typename StringType, typename FragmentStringType = StringType>
	bool ends_with(const StringType & str, const FragmentStringType & fragment) {
		auto strSize = size(str);
		auto fragmentSize = size(fragment);

		if(fragmentSize > strSize) {
			return false;
		}

		// using forward rather than reverse iterators ensures compatibility with null-terminated
		// [const] char *
		auto strIt = cbegin(str) + strSize - fragmentSize;
		auto fragmentIt = cbegin(fragment);
		const auto end = cend(fragment);

		while(fragmentIt != end) {
			if(*strIt != *fragmentIt) {
				return false;
			}

			++strIt;
			++fragmentIt;
		}

		return true;
	}


	template<typename IntType = int, typename StringType, typename = std::enable_if_t<std::is_integral<IntType>::value>, typename = std::enable_if_t<std::is_signed<IntType>::value>>
	std::enable_if_t<std::is_same<std::decay_t<StringType>, char *>::value || std::is_same<std::decay_t<StringType>, const char *>::value, std::optional<IntType>>
	parse_int(StringType str, int base = 10) {
		char * end;
		auto ret = strtoll(str, &end, base);

		if(str == end) {
			return {};
		}

		while(*end && std::isspace(*end)) {
			++end;
		}

		if('\0' != *end) {
			return {};
		}

		if(std::numeric_limits<IntType>::min() > ret || std::numeric_limits<IntType>::max() < ret) {
			return {};
		}

		return static_cast<IntType>(ret);
	}


	template<typename IntType = int, typename StringType, typename = std::enable_if_t<std::is_integral<IntType>::value>, typename = std::enable_if_t<std::is_unsigned<IntType>::value>>
	std::enable_if_t<std::is_same<std::decay_t<StringType>, char *>::value || std::is_same<std::decay_t<StringType>, const char *>::value, std::optional<IntType>>
	parse_uint(StringType str, int base = 10) {
		char * end;
		auto ret = strtoull(str, &end, base);

		if(str == end) {
			return {};
		}

		while(*end && std::isspace(*end)) {
			++end;
		}

		if('\0' != *end) {
			return {};
		}

		if(std::numeric_limits<IntType>::max() < ret) {
			return {};
		}

		return static_cast<IntType>(ret);
	}


	template<typename IntType = int, typename StringType = std::string>
	std::optional<IntType> parse_int(const StringType & str, int base = 10) {
		return parse_int<IntType, const char *>(str.data(), base);
	}


	template<typename IntType = int, typename StringType = std::string>
	std::optional<IntType> parse_unt(const StringType & str, int base = 10) {
		return parse_uint<IntType, const char *>(str.data(), base);
	}


	template<typename StringType, bool doQuotes = false>
	StringType to_html_entities(const StringType & str) {
		StringType ret;
		typename StringType::size_type capacity = str.size() * Detail::EscapeBufferSizeFactor;

		if(capacity > ret.capacity()) {
			ret.reserve(capacity);
		}

		for(const auto & ch : str) {
			if constexpr(doQuotes) {
				if('"' == ch) {
					ret.append("&quot;");
					continue;
				}
				else if('\'' == ch) {
					ret.append("&#039;");  // like &apoos; but with wider compatibility
					continue;
				}
			}

			if('<' == ch) {
				ret.append("&lt;");
			}
			else if('>' == ch) {
				ret.append("&gt;");
			}
			else if('&' == ch) {
				ret.append("&amp;");
			}
			else {
				ret.push_back(ch);
			}
		}

		return ret;
	}


	// this is basic, naive percent-decode. it doesn't identify invalid %-sequences
	template<typename StringType>
	StringType percent_decode(const StringType & str) {
		std::basic_regex rxPercent(StringType("%([0-9a-f]{2})"), std::regex::icase);

		auto rxIter = std::regex_iterator(str.begin(), str.end(), rxPercent);
		const auto end = decltype(rxIter)();

		if(rxIter == end) {
			return str;
		}

		StringType ret;

		// we know it will be at most the same length as str, so make sure no appends
		// require reallocation
		if(ret.capacity() < str.size()) {
			ret.reserve(str.size());
		}

		int copyStartOffset = 0;

		for(; rxIter != end; ++rxIter) {
			// match is guaranteed to be 2 hex digits so won't overflow size of char
			const char ch = static_cast<char>(std::strtol((*rxIter)[1].str().data(), nullptr, 16));
			const auto copyEndOffset = rxIter->position(0);
			ret.append(str, copyStartOffset, copyEndOffset - copyStartOffset);
			ret.push_back(ch);
			copyStartOffset = copyEndOffset + 3;
		}

		ret.append(str, copyStartOffset, str.size() - copyStartOffset);
		return ret;
	};


	// this is specifically focused on percent-encoding for URIs
	template<typename StringType>
	StringType percent_encode(const StringType & str) {
		StringType ret;

		std::transform(str.cbegin(), str.cend(), std::back_inserter(str), [&ret](const typename StringType::char_type ch) {
			// using switch() wouldn't work for non-itegral char types (e.g. QChar)
			if('\n' == ch) {
				ret.append("%0A");
			}
			else if('\r' == ch) {
				ret.append("%0D");
			}
			else if(' ' == ch) {
				ret.append("%20");
			}
			else if('!' == ch) {
				ret.append("%21");
			}
			else if('#' == ch) {
				ret.append("%23");
			}
			else if('$' == ch) {
				ret.append("%24");
			}
			else if('%' == ch) {
				ret.append("%25");
			}
			else if('&' == ch) {
				ret.append("%26");
			}
			else if('\'' == ch) {
				ret.append("%27");
			}
			else if('(' == ch) {
				ret.append("%28");
			}
			else if(')' == ch) {
				ret.append("%29");
			}
			else if('*' == ch) {
				ret.append("%2A");
			}
			else if('+' == ch) {
				ret.append("%2B");
			}
			else if(',' == ch) {
				ret.append("%2C");
			}
			else if('/' == ch) {
				ret.append("%2F");
			}
			else if(':' == ch) {
				ret.append("%3A");
			}
			else if(';' == ch) {
				ret.append("%3B");
			}
			else if('=' == ch) {
				ret.append("%3D");
			}
			else if('?' == ch) {
				ret.append("%3F");
			}
			else if('@' == ch) {
				ret.append("%40");
			}
			else if('[' == ch) {
				ret.append("%5B");
			}
			else if(']' == ch) {
				ret.append("%5D");
			}
			else {
				ret.push_back(ch);
			}
		});

		return ret;
	}


}  // namespace Equit


#endif  // ANANSI_STRINGS_H
