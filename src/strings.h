#ifndef EQUITWEBSERVER_STRINGS_H
#define EQUITWEBSERVER_STRINGS_H

#include <iterator>
#include <algorithm>
#include <regex>
#include <cctype>


namespace EquitWebServer {
	template<typename T>
	T to_lower(const T & str) {
		T ret;

		std::transform(str.cbegin(), str.cend(), std::back_inserter(ret), [](const auto & ch) {
			return std::tolower(ch);
		});

		return ret;
	};


	template<typename T, bool doQuotes = false>
	T html_escape(const T & str) {
		T ret;
		auto capacity = str.size() * 1.1;

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
					ret.append("&apos;");  // TODO &#039; for broader compatibility?
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


	// TODO this is basic, naive percent-decode. it doesn't identify invalid %-sequences
	template<typename T>
	T percent_decode(const T & str) {
		std::basic_regex rxPercent(T("%([0-9a-f]{2})"), std::regex::icase);

		auto begin = std::regex_iterator(str.begin(), str.end(), rxPercent);
		const auto end = decltype(begin)();

		if(begin == end) {
			return str;
		}

		T ret;

		// we know it will be at most the same length as str, so make sure no appends
		// require reallocation
		if(ret.capacity() < str.size()) {
			ret.reserve(str.size());
		}

		int copyStartOffset = 0;

		for(auto & iter = begin; iter != end; ++iter) {
			const char ch = static_cast<char>(std::strtol((*iter)[1].str().data(), nullptr, 16));
			const auto copyEndOffset = iter->position(0);
			ret.append(str, copyStartOffset, copyEndOffset - copyStartOffset);
			ret.push_back(ch);
			copyStartOffset = copyEndOffset + 3;
		}

		ret.append(str, copyStartOffset, str.size() - copyStartOffset);
		return ret;
	};
}  // namespace EquitWebServer


#endif  // EQUITWEBSERVER_STRINGS_H
