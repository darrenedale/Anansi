/// \file deflater.h
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Declaration of the Deflater class for Equit.
///
/// \dep
/// - <string>
/// - <istream>
/// - <ostream>
/// - <optional>
/// - <zlib.h>
///
/// \par Changes
/// - (2018-03) First release.

#ifndef EQUIT_DEFLATER_H
#define EQUIT_DEFLATER_H

#include <string>
#include <istream>
#include <ostream>
#include <optional>

#include <zlib.h>

namespace Equit {

	class Deflater {
	public:
		enum class HeaderType {
			Deflate = 0,
			Gzip,
			None,
		};

		explicit Deflater(int compressionLevel = -1);
		explicit Deflater(HeaderType type, int compressionLevel = -1);
		virtual ~Deflater();

		void reset();
		std::string addData(const std::string & data);
		std::optional<std::string> addData(std::istream & in, const std::optional<int> & size = {});
		std::size_t addDataTo(std::ostream & out, const std::string & data);
		std::optional<std::size_t> addDataTo(std::ostream & out, std::istream & in, const std::optional<int> & size = {});
		std::string finish();
		std::size_t finish(std::ostream & out);

		static inline std::string deflate(const std::string & data, int compressionLevel = -1) {
			Deflater deflater(compressionLevel);
			std::string ret = deflater.addData(data);
			const auto finalData = deflater.finish();
			std::copy(finalData.cbegin(), finalData.cend(), std::back_inserter(ret));
			return ret;
		}

		static inline std::string deflate(std::istream & in, int compressionLevel = -1, const std::optional<int> & size = {}) {
			Deflater deflater(compressionLevel);
			std::string ret = *deflater.addData(in, size);
			const auto finalData = deflater.finish();
			std::copy(finalData.cbegin(), finalData.cend(), std::back_inserter(ret));
			return ret;
		}

		static inline std::size_t deflateTo(std::ostream & out, const std::string & data, int compressionLevel = -1) {
			Deflater deflater(compressionLevel);
			std::size_t ret = deflater.addDataTo(out, data);
			ret += deflater.finish(out);
			return ret;
		}

		static inline std::size_t deflateTo(std::ostream & out, std::istream & in, int compressionLevel = -1, const std::optional<int> & size = {}) {
			Deflater deflater(compressionLevel);
			std::size_t ret = *deflater.addDataTo(out, in, size);
			ret += deflater.finish(out);
			return ret;
		}

	private:
		z_stream m_zStream;
	};

}  // namespace Equit

#endif  // EQUIT_DEFLATER_H
