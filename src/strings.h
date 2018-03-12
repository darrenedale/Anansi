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
/// \par Changes
/// - (2018-03) First release.

#ifndef EQUIT_STRINGS_H
#define EQUIT_STRINGS_H

#include <iterator>
#include <algorithm>
#include <regex>
#include <cctype>


namespace Equit {


	template<typename StringType>
	StringType to_lower(const StringType & str) {
		StringType ret;

		std::transform(str.cbegin(), str.cend(), std::back_inserter(ret), [](const auto & ch) {
			return std::tolower(ch);
		});

		return ret;
	};


	template<typename StringType>
	StringType to_upper(const StringType & str) {
		StringType ret;

		std::transform(str.cbegin(), str.cend(), std::back_inserter(ret), [](const auto & ch) {
			return std::toupper(ch);
		});

		return ret;
	};


	template<typename StringType, typename FragmentStringType = StringType>
	bool starts_with(const StringType & str, const FragmentStringType & fragment) {
		if(fragment.size() > str.size()) {
			return false;
		}

		auto strIt = str.cbegin();
		auto fragmentIt = fragment.cbegin();
		const auto end = fragment.cend();

		while(fragmentIt != end) {
			if(*strIt != *fragmentIt) {
				return false;
			}

			++strIt;
			++fragmentIt;
		}

		return true;
	}


	template<typename StringType, bool doQuotes = false>
	StringType to_html_entities(const StringType & str) {
		StringType ret;
		// this capacity is just an estimate
		typename StringType::size_type capacity = str.size() * 1.1;

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
