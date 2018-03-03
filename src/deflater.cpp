#include "deflater.h"

#include <iostream>
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


	std::string Deflater::addData(const std::string & data) {
		m_zStream.avail_in = static_cast<unsigned int>(data.size());
		m_zStream.next_in = reinterpret_cast<unsigned char *>(const_cast<char *>(data.data()));
		std::array<unsigned char, ChunkSize> outBuffer;
		std::string ret;
		const auto begin = outBuffer.cbegin();

		do {
			m_zStream.avail_out = outBuffer.size();
			m_zStream.next_out = &outBuffer[0];
			int res = ::deflate(&m_zStream, Z_NO_FLUSH);  // no bad return value
			assert(res != Z_STREAM_ERROR);					 // state not clobbered
			ret.append(begin, begin + (outBuffer.size() - m_zStream.avail_out));
		} while(0 == m_zStream.avail_out);

		assert(0 == m_zStream.avail_in);  // all input will be used
		return ret;
	}


	std::optional<std::string> Deflater::addData(std::istream & in, const optional_int & size) {
		std::array<unsigned char, ChunkSize> inBuffer;
		std::array<unsigned char, ChunkSize> outBuffer;
		int bytesRead = 0;
		std::string ret;
		const auto begin = outBuffer.cbegin();

		while(!in.eof() && (!size || bytesRead < *size)) {
			if(!in.read(reinterpret_cast<char *>(&inBuffer[0]), ChunkSize)) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to read from input stream\n";
				return {};
			}

			m_zStream.avail_in = static_cast<unsigned int>(in.gcount());
			bytesRead += m_zStream.avail_in;
			m_zStream.next_in = &inBuffer[0];

			do {
				m_zStream.avail_out = outBuffer.size();
				m_zStream.next_out = &outBuffer[0];
				int res = ::deflate(&m_zStream, Z_NO_FLUSH);  // no bad return value
				assert(res != Z_STREAM_ERROR);					 // state not clobbered
				ret.append(begin, begin + (outBuffer.size() - m_zStream.avail_out));
			} while(0 == m_zStream.avail_out);
		}

		assert(0 == m_zStream.avail_in);  // all input will be used
		return ret;
	}


	std::size_t Deflater::addDataTo(std::ostream & out, const std::string & data) {
		m_zStream.avail_in = static_cast<unsigned int>(data.size());
		m_zStream.next_in = reinterpret_cast<unsigned char *>(const_cast<char *>(data.data()));
		std::array<unsigned char, ChunkSize> outBuffer;
		std::size_t ret;

		do {
			m_zStream.avail_out = outBuffer.size();
			m_zStream.next_out = &outBuffer[0];
			int res = ::deflate(&m_zStream, Z_NO_FLUSH);  // no bad return value
			assert(res != Z_STREAM_ERROR);					 // state not clobbered
			auto deflatedSize = outBuffer.size() - m_zStream.avail_out;
			ret += deflatedSize;
			out.write(reinterpret_cast<char *>(&outBuffer[0]), static_cast<std::streamsize>(deflatedSize));
		} while(0 == m_zStream.avail_out);

		assert(0 == m_zStream.avail_in);  // all input will be used
		return ret;
	}


	std::optional<std::size_t> Deflater::addDataTo(std::ostream & out, std::istream & in, const optional_int & size) {
		std::array<unsigned char, ChunkSize> inBuffer;
		std::array<unsigned char, ChunkSize> outBuffer;
		int bytesRead = 0;
		std::size_t ret;

		while(!in.eof() && (!size || bytesRead < *size)) {
			if(!in.read(reinterpret_cast<char *>(&inBuffer[0]), ChunkSize)) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to read from input stream\n";
				return {};
			}

			m_zStream.avail_in = static_cast<unsigned int>(in.gcount());
			m_zStream.next_in = &inBuffer[0];
			bytesRead += m_zStream.avail_in;

			do {
				m_zStream.avail_out = outBuffer.size();
				m_zStream.next_out = &outBuffer[0];
				int res = ::deflate(&m_zStream, Z_NO_FLUSH);  // no bad return value
				assert(res != Z_STREAM_ERROR);					 // state not clobbered
				auto deflatedSize = outBuffer.size() - m_zStream.avail_out;
				ret += deflatedSize;

				if(!out.write(reinterpret_cast<char *>(&outBuffer[0]), static_cast<std::streamsize>(deflatedSize))) {
					return {};
				}
			} while(0 == m_zStream.avail_out);
		}

		assert(0 == m_zStream.avail_in);  // all input will be used
		return ret;
	}


	std::string Deflater::finish() {
		std::array<unsigned char, ChunkSize> outBuffer;
		std::string ret;
		const auto begin = outBuffer.cbegin();
		int res;

		do {
			m_zStream.avail_out = outBuffer.size();
			m_zStream.next_out = &outBuffer[0];
			res = ::deflate(&m_zStream, Z_FINISH);  // no bad return value
			assert(res != Z_STREAM_ERROR);			 // state not clobbered
			ret.append(begin, begin + (outBuffer.size() - m_zStream.avail_out));
		} while(0 == m_zStream.avail_out);

		assert(0 == m_zStream.avail_in);  // all input will be used
		assert(Z_STREAM_END == res);		 // stream will be complete
		return ret;
	}


	std::size_t Deflater::finish(std::ostream & out) {
		std::array<unsigned char, ChunkSize> outBuffer;
		std::size_t ret = 0;
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


	void Deflater::reset() {
		auto res = deflateReset(&m_zStream);

		if(Z_OK != res) {
			throw;
		}
	}


}  // namespace Equit
