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
		Deflater(int compressionLevel);
		virtual ~Deflater();

		std::string deflate(const std::string & data);
		std::string deflate(std::istream & in, std::optional<int> size = {});
		std::size_t deflateTo(std::ostream & out, const std::string & data);
		std::size_t deflateTo(std::ostream & out, std::istream & in, std::optional<int> size = {});

		void reset(int compressionLevel);

	private:
		z_stream m_zStream;
	};

}  // namespace Equit

#endif  // EQUIT_DEFLATER_H
