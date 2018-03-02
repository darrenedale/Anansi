#include "deflater.h"

#include <array>
#include <cassert>

#include <zlib.h>

namespace Equit {


	static constexpr const uint32_t ChunkSize = 1024;


	Deflater::Deflater(int compressionLevel) {
		m_zStream.zalloc = nullptr;
		m_zStream.zfree = nullptr;
		m_zStream.opaque = nullptr;
		auto ret = deflateInit(&m_zStream, compressionLevel);

		if(ret != Z_OK) {
			throw;
		}
	}


	std::string Deflater::deflate(const std::string & data) {
		auto size = data.size();
		m_zStream.avail_in = static_cast<unsigned int>(size);
		m_zStream.next_in = reinterpret_cast<unsigned char *>(const_cast<char *>(data.data()));
		std::string ret;
		std::array<unsigned char, ChunkSize> outBuffer;
		int res;

		do {
			m_zStream.avail_out = outBuffer.size();
			m_zStream.next_out = &outBuffer[0];
			res = ::deflate(&m_zStream, Z_FINISH);  // no bad return value
			assert(res != Z_STREAM_ERROR);			 // state not clobbered
			ret.append(outBuffer.cbegin(), outBuffer.cend());
		} while(0 == m_zStream.avail_out);

		assert(0 == m_zStream.avail_in);  // all input will be used
		assert(Z_STREAM_END == res);		 // stream will be complete

		return ret;
	}


	std::size_t Deflater::deflateTo(std::ostream & out, const std::string & data) {
		auto size = data.size();
		m_zStream.avail_in = static_cast<unsigned int>(size);
		m_zStream.next_in = reinterpret_cast<unsigned char *>(const_cast<char *>(data.data()));
		std::array<unsigned char, ChunkSize> outBuffer;
		std::size_t ret;
		int res;

		do {
			m_zStream.avail_out = outBuffer.size();
			m_zStream.next_out = &outBuffer[0];
			res = ::deflate(&m_zStream, Z_FINISH);  // no bad return value
			assert(res != Z_STREAM_ERROR);			 // state not clobbered
			auto deflatedSize = outBuffer.size() - m_zStream.avail_out;
			ret += deflatedSize;
			out.write(reinterpret_cast<char *>(&outBuffer[0]), static_cast<std::streamsize>(deflatedSize));
		} while(0 == m_zStream.avail_out);

		assert(0 == m_zStream.avail_in);  // all input will be used
		assert(Z_STREAM_END == res);		 // stream will be complete

		return ret;
	}


	Deflater::~Deflater() {
		deflateEnd(&m_zStream);
	}


	void Deflater::reset(int compressionLevel) {
		deflateEnd(&m_zStream);
		m_zStream.zalloc = nullptr;
		m_zStream.zfree = nullptr;
		m_zStream.opaque = nullptr;
		deflateInit(&m_zStream, compressionLevel);
	}


}  // namespace Equit
