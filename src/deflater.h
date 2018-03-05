#ifndef EQUIT_DEFLATER_H
#define EQUIT_DEFLATER_H

#include <string>
#include <istream>
#include <ostream>
#include <optional>

#include <zlib.h>

namespace Equit {

	using optional_int = std::optional<int>;

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
		std::optional<std::string> addData(std::istream & in, const optional_int & size = {});
		std::size_t addDataTo(std::ostream & out, const std::string & data);
		std::optional<std::size_t> addDataTo(std::ostream & out, std::istream & in, const optional_int & size = {});
		std::string finish();
		std::size_t finish(std::ostream & out);

		static inline std::string deflate(const std::string & data, int compressionLevel = -1) {
			std::string ret;
			Deflater deflater(compressionLevel);
			ret.append(deflater.addData(data));
			ret.append(deflater.finish());
			return ret;
		}

		static inline std::string deflate(std::istream & in, int compressionLevel = -1, const optional_int & size = {}) {
			std::string ret;
			Deflater deflater(compressionLevel);
			ret.append(*deflater.addData(in, size));
			ret.append(deflater.finish());
			return ret;
		}

		static inline std::size_t deflateTo(std::ostream & out, const std::string & data, int compressionLevel = -1) {
			std::size_t ret;
			Deflater deflater(compressionLevel);
			ret += deflater.addDataTo(out, data);
			ret += deflater.finish(out);
			return ret;
		}

		static inline std::size_t deflateTo(std::ostream & out, std::istream & in, int compressionLevel = -1, const optional_int & size = {}) {
			std::size_t ret;
			Deflater deflater(compressionLevel);
			ret += *deflater.addDataTo(out, in, size);
			ret += deflater.finish(out);
			return ret;
		}

	private:
		z_stream m_zStream;
	};

}  // namespace Equit

#endif  // EQUIT_DEFLATER_H
