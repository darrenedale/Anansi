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

/// \file zlibdeflater.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of the ZLibDeflater class template for Equit.
///
/// \dep
/// - <stdexcept>
/// - <iostream>
/// - <array>
/// - <string>
/// - <istream>
/// - <ostream>
/// - <optional>
/// - <zlib.h>
/// - "macros.h"
/// - "assert.h"
///
/// \par Changes
/// - (2018-03) First release.

#ifndef EQ_ZLIBDEFLATER_H
#define EQ_ZLIBDEFLATER_H

#include <stdexcept>
#include <iostream>
#include <array>
#include <string>
#include <istream>
#include <ostream>
#include <optional>

#include <zlib.h>

#include "macros.h"
#include "assert.h"

namespace Equit {


	namespace Detail {
		std::optional<int64_t> stdioRead(std::istream & in, char * data, int64_t max);
		std::optional<int64_t> stdioWrite(std::ostream & out, const char * data, int64_t size);
		bool stdioEof(std::istream & in);
	}  // namespace Detail


	// implement functions that match this signature to use Deflater with streams that provide different
	// interfaces from the one provided by c++ io streams
	template<class InStream = std::istream>
	using ZLibDeflaterReadFunction = std::optional<int64_t> (*)(InStream &, char *, int64_t);

	template<class InStream = std::istream>
	using ZLibDeflaterWriteFunction = std::optional<int64_t> (*)(InStream &, char *, int64_t);

	template<class InStream = std::istream>
	using ZLibDeflaterStreamEndFunction = bool (*)(const InStream &);


	// outside class for convenience - doesn't depend on any template params and
	// avoids having to write out full template args every time it's used
	enum class ZLibDeflaterHeaderType {
		Deflate = 0,
		Gzip,
		None,
	};


	template<class ByteArray = std::string, class OutStream = std::ostream, class InStream = std::istream, ZLibDeflaterReadFunction<InStream> readFromStream = Detail::stdioRead, ZLibDeflaterWriteFunction<OutStream> writeToStream = Detail::stdioWrite, ZLibDeflaterStreamEndFunction<InStream> streamEof = Detail::stdioEof, typename ByteArraySizeType = typename ByteArray::size_type>
	class ZLibDeflater {
	public:
		using ByteArrayType = ByteArray;
		using SizeType = ByteArraySizeType;
		using OutputStreamType = OutStream;
		using InputStreamType = InStream;

		using HeaderType = ZLibDeflaterHeaderType;


		static constexpr const int DefaultCompressionLevel = Z_DEFAULT_COMPRESSION;


		explicit ZLibDeflater(int compressionLevel = DefaultCompressionLevel)
		: ZLibDeflater(HeaderType::Deflate, compressionLevel) {
		}


		explicit ZLibDeflater(HeaderType type, int compressionLevel = DefaultCompressionLevel) {
			m_zStream.zalloc = nullptr;
			m_zStream.zalloc = nullptr;
			m_zStream.zfree = nullptr;
			m_zStream.opaque = nullptr;
			auto windowBits = DeflateWindowBits;

			switch(type) {
				case HeaderType::Deflate:
					// this is the default, set above
					break;

				case HeaderType::Gzip:
					windowBits = GzipWindowBits;
					break;

				case HeaderType::None:
					windowBits = RawWindowBits;
					break;
			}

			auto result = ::deflateInit2(&m_zStream, compressionLevel, Z_DEFLATED, windowBits, DefaultMemoryLevel, Z_DEFAULT_STRATEGY);
			m_zStream.avail_in = 0;

			if(Z_OK != result) {
				throw std::runtime_error("failed to initialise zlib stream");
			}
		}


		~ZLibDeflater() {
			deflateEnd(&m_zStream);
		}


		void reset() {
			auto res = deflateReset(&m_zStream);

			if(Z_OK != res) {
				throw std::runtime_error("failed to reset zlib stream");
			}
		}


		ByteArray addData(const ByteArray & data) {
			m_zStream.avail_in = static_cast<unsigned int>(data.size());
			m_zStream.next_in = reinterpret_cast<unsigned char *>(const_cast<char *>(data.data()));
			std::array<unsigned char, ChunkSize> outBuffer;
			ByteArray ret;
			const auto begin = outBuffer.cbegin();

			do {
				m_zStream.avail_out = outBuffer.size();
				m_zStream.next_out = &outBuffer[0];
				int result = ::deflate(&m_zStream, Z_NO_FLUSH);
				eqAssert(Z_STREAM_ERROR != result, "failed to deflate " << data.size() << " bytes of data");
				std::copy(begin, begin + (outBuffer.size() - m_zStream.avail_out), std::back_inserter(ret));
			} while(0 == m_zStream.avail_out);

			eqAssert(0 == m_zStream.avail_in, "failed to deflate " << data.size() << " bytes of data (failed to exhaust input buffer, still contains " << m_zStream.avail_in << "  bytes)");
			return ret;
		}


		std::optional<ByteArray> addData(InStream & in, const std::optional<int64_t> & size = {}) {
			std::array<unsigned char, ChunkSize> inBuffer;
			std::array<unsigned char, ChunkSize> outBuffer;
			int64_t bytesRead = 0;
			ByteArray ret;
			const auto begin = outBuffer.cbegin();

			while(!streamEof(in) && (!size || bytesRead < *size)) {
				auto thisRead = readFromStream(in, reinterpret_cast<char *>(&inBuffer[0]), ChunkSize);

				if(!thisRead) {
					std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: failed to read from input stream\n";
					return {};
				}

				m_zStream.avail_in = static_cast<unsigned int>(*thisRead);
				bytesRead += m_zStream.avail_in;
				m_zStream.next_in = &inBuffer[0];

				do {
					m_zStream.avail_out = outBuffer.size();
					m_zStream.next_out = &outBuffer[0];
					auto result = ::deflate(&m_zStream, Z_NO_FLUSH);
					eqAssert(Z_STREAM_ERROR != result, "failed to deflate data from input stream (input buffer contains " << m_zStream.avail_in << " bytes, output buffer has space for " << outBuffer.size() << " bytes)");
					std::copy(begin, begin + (outBuffer.size() - m_zStream.avail_out), std::back_inserter(ret));
				} while(0 == m_zStream.avail_out);
			}

			eqAssert(0 == m_zStream.avail_in, "failed to deflate data from input stream (failed to exhaust input buffer, still contains " << m_zStream.avail_in << "  bytes)");
			return ret;
		}


		std::optional<SizeType> addDataTo(OutStream & out, const ByteArray & data) {
			m_zStream.avail_in = static_cast<unsigned int>(data.size());
			m_zStream.next_in = reinterpret_cast<unsigned char *>(const_cast<char *>(data.data()));
			std::array<unsigned char, ChunkSize> outBuffer;
			SizeType ret = 0;

			do {
				m_zStream.avail_out = outBuffer.size();
				m_zStream.next_out = &outBuffer[0];
				auto result = ::deflate(&m_zStream, Z_NO_FLUSH);
				eqAssert(Z_STREAM_ERROR != result, "failed to deflate " << data.size() << " bytes of data (input buffer contains " << m_zStream.avail_in << " bytes, output buffer has space for " << outBuffer.size() << " bytes)");
				auto deflatedSize = outBuffer.size() - m_zStream.avail_out;
				ret += deflatedSize;
				char * deflatedData = reinterpret_cast<char *>(&outBuffer[0]);

				do {
					auto thisWrite = writeToStream(out, deflatedData, deflatedSize);

					if(!thisWrite) {
						return {};
					}

					deflatedSize -= *thisWrite;
					deflatedData += *thisWrite;
				} while(0 < deflatedSize);
			} while(0 == m_zStream.avail_out);

			eqAssert(0 == m_zStream.avail_in, "failed to deflate " << data.size() << " bytes of data (failed to exhaust input buffer, still contains " << m_zStream.avail_in << "  bytes)");
			return ret;
		}


		std::optional<SizeType> addDataTo(OutStream & out, InStream & in, const std::optional<int64_t> & size = {}) {
			std::array<unsigned char, ChunkSize> inBuffer;
			std::array<unsigned char, ChunkSize> outBuffer;
			int bytesRead = 0;
			std::size_t ret = 0;

			while(!streamEof(in) && (!size || bytesRead < *size)) {
				auto thisRead = readFromStream(in, reinterpret_cast<char *>(&inBuffer[0]), ChunkSize);

				if(!thisRead) {
					std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: failed to read from input stream\n";
					return {};
				}

				m_zStream.avail_in = static_cast<unsigned int>(*thisRead);
				m_zStream.next_in = &inBuffer[0];
				bytesRead += m_zStream.avail_in;

				do {
					m_zStream.avail_out = outBuffer.size();
					m_zStream.next_out = &outBuffer[0];
					auto result = ::deflate(&m_zStream, Z_NO_FLUSH);
					eqAssert(Z_STREAM_ERROR != result, "failed to deflate data from input stream (input buffer contains " << m_zStream.avail_in << " bytes, output buffer has space for " << outBuffer.size() << " bytes)");
					auto deflatedSize = outBuffer.size() - m_zStream.avail_out;
					ret += deflatedSize;
					char * deflatedData = reinterpret_cast<char *>(&outBuffer[0]);

					do {
						auto thisWrite = writeToStream(out, deflatedData, deflatedSize);

						if(!thisWrite) {
							return {};
						}

						deflatedSize -= *thisWrite;
						deflatedData += *thisWrite;
					} while(0 < deflatedSize);
				} while(0 == m_zStream.avail_out);
			}

			eqAssert(0 == m_zStream.avail_in, "failed to deflate data from input stream (failed to exhaust input buffer, still contains " << m_zStream.avail_in << "  bytes)");
			return ret;
		}


		ByteArray finish() {
			std::array<unsigned char, ChunkSize> outBuffer;
			ByteArray ret;
			const auto begin = outBuffer.cbegin();
			int result;

			do {
				m_zStream.avail_out = outBuffer.size();
				m_zStream.next_out = &outBuffer[0];
				result = ::deflate(&m_zStream, Z_FINISH);
				eqAssert(Z_STREAM_ERROR != result, "failed to finish deflating (deflate() returned Z_STREAM_ERROR)");
				std::copy(begin, begin + (outBuffer.size() - m_zStream.avail_out), std::back_inserter(ret));
			} while(0 == m_zStream.avail_out);

			eqAssert(0 == m_zStream.avail_in, "failed to finish deflating (failed to exhaust input buffer, still contains " << m_zStream.avail_in << "  bytes)");
			eqAssert(Z_STREAM_END == result, "failed to finish deflating (result is " << result << ", expecting Z_STREAM_END [" << Z_STREAM_END << "])");
			return ret;
		}


		std::optional<SizeType> finish(OutStream & out) {
			std::array<unsigned char, ChunkSize> outBuffer;
			SizeType ret = 0;
			int result;

			do {
				m_zStream.avail_out = outBuffer.size();
				m_zStream.next_out = &outBuffer[0];
				result = ::deflate(&m_zStream, Z_FINISH);
				eqAssert(Z_STREAM_ERROR != result, "failed to finish deflating (deflate() returned Z_STREAM_ERROR)");
				auto deflatedSize = outBuffer.size() - m_zStream.avail_out;
				ret += deflatedSize;
				char * deflatedData = reinterpret_cast<char *>(&outBuffer[0]);

				do {
					auto thisWrite = writeToStream(out, deflatedData, deflatedSize);

					if(!thisWrite) {
						return {};
					}

					deflatedSize -= *thisWrite;
					deflatedData += *thisWrite;
				} while(0 < deflatedSize);
			} while(0 == m_zStream.avail_out);

			eqAssert(0 == m_zStream.avail_in, "failed to finish deflating (failed to exhaust input buffer, still contains " << m_zStream.avail_in << "  bytes)");
			eqAssert(Z_STREAM_END == result, "failed to finish deflating (result is " << result << ", expecting Z_STREAM_END [" << Z_STREAM_END << "])");
			return ret;
		}


		static inline ByteArray deflate(const ByteArray & data, int compressionLevel = DefaultCompressionLevel) {
			ZLibDeflater deflater(compressionLevel);
			ByteArray ret = deflater.addData(data);
			const auto finalData = deflater.finish();
			std::copy(finalData.cbegin(), finalData.cend(), std::back_inserter(ret));
			return ret;
		}


		static inline std::optional<ByteArray> deflate(InStream & in, int compressionLevel = DefaultCompressionLevel, const std::optional<int64_t> & size = {}) {
			ZLibDeflater deflater(compressionLevel);
			auto ret = deflater.addData(in, size);

			if(!ret) {
				return ret;
			}

			const auto finalData = deflater.finish();
			std::copy(finalData.cbegin(), finalData.cend(), std::back_inserter(*ret));
			return ret;
		}


		static inline std::optional<SizeType> deflateTo(OutStream & out, const ByteArray & data, int compressionLevel = DefaultCompressionLevel) {
			ZLibDeflater deflater(compressionLevel);
			SizeType ret = 0;
			auto result = deflater.addDataTo(out, data);

			if(!result) {
				return result;
			}

			ret += *result;
			result = deflater.finish(out);

			if(!result) {
				return result;
			}

			return ret + *result;
		}


		static inline std::optional<SizeType> deflateTo(OutStream & out, InStream & in, int compressionLevel = DefaultCompressionLevel, const std::optional<int64_t> & size = {}) {
			ZLibDeflater deflater(compressionLevel);
			SizeType ret = 0;
			auto result = deflater.addDataTo(out, in, size);

			if(!result) {
				return result;
			}

			ret += *result;
			result = deflater.finish(out);

			if(!result) {
				return result;
			}

			return ret + *result;
		}


	private:
		static constexpr const uint64_t ChunkSize = 16384;
		static constexpr const int DefaultMemoryLevel = 8;  // 1 - 9 (1 minimises usage, 9 maximises; 8 is the default used by ::deflateInit()
		static constexpr const int DeflateWindowBits = 15;  // 0 - 15 produces a deflate stream
		static constexpr const int GzipWindowBits = 31;		 // 16 or greater produces a gzip stream
		static constexpr const int RawWindowBits = -15;		 // -8 - -15 produces a headerless stream

		z_stream m_zStream;
	};

}  // namespace Equit

#endif  // EQ_ZLIBDEFLATER_H
