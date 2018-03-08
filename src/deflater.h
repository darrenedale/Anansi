/// \file deflater.h
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Declaration of the Deflater class for Equit.
///
/// \dep
/// - <cassert>
/// - <stdexcept>
/// - <iostream>
/// - <array>
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

#include <cassert>
#include <stdexcept>
#include <iostream>
#include <array>
#include <string>
#include <istream>
#include <ostream>
#include <optional>

#include <zlib.h>

namespace Equit {

	// TODO make these a templated stream handler class?
	template<class StreamType = std::istream>
	static std::optional<int64_t> deflaterDefaultRead(StreamType & in, char * data, int64_t max) {
		if(!in.read(data, max)) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to read from input stream\n";
			return {};
		}

		return in.gcount();
	}

	template<class StreamType = std::ostream>
	static std::optional<int64_t> deflaterDefaultWrite(StreamType & out, const char * data, int64_t size) {
		assert(0 <= size);

		if(0 == size) {
			return 0;
		}

		const auto pos = out.tellp();

		if(!out.write(data, size)) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to read from input stream\n";
			return {};
		}

		return out.tellp() - pos;
	}

	template<class StreamType = std::istream>
	static bool deflaterDefaultStreamEnd(StreamType & in) {
		return in.eof();
	}

	// create specialisations of these to use Deflater with streams that provide different
	// interfaces from the one provided by c++ io streams
	template<class InStream = std::istream>
	using DeflaterReadFunction = std::optional<int64_t> (*)(InStream &, char *, int64_t);

	template<class InStream = std::istream>
	using DeflaterWriteFunction = std::optional<int64_t> (*)(InStream &, char *, int64_t);

	template<class InStream = std::istream>
	using DeflaterStreamEndFunction = bool (*)(const InStream &);

	template<class ByteArray = std::string, typename ByteArraySizeType = typename ByteArray::size_type, class OutStream = std::ostream, class InStream = std::istream, DeflaterReadFunction<InStream> readFromStream = deflaterDefaultRead, DeflaterReadFunction<InStream> writeToStream = deflaterDefaultWrite, DeflaterStreamEndFunction<InStream> streamEof = deflaterDefaultStreamEnd>
	class Deflater {
	public:
		using ByteArrayType = ByteArray;
		using SizeType = ByteArraySizeType;
		using OutputStreamType = OutStream;
		using InputStreamType = InStream;

		enum class HeaderType {
			Deflate = 0,
			Gzip,
			None,
		};


		explicit Deflater(int compressionLevel = -1)
		: Deflater(HeaderType::Deflate, compressionLevel) {
		}

		explicit Deflater(HeaderType type, int compressionLevel = -1) {
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

			auto result = ::deflateInit2(&m_zStream, compressionLevel, Z_DEFLATED, windowBits, 8, Z_DEFAULT_STRATEGY);
			m_zStream.avail_in = 0;

			if(Z_OK != result) {
				throw std::runtime_error("failed to initialise zlib stream");
			}
		}


		~Deflater() {
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
				assert(Z_STREAM_ERROR != result);
				std::copy(begin, begin + (outBuffer.size() - m_zStream.avail_out), std::back_inserter(ret));
			} while(0 == m_zStream.avail_out);

			assert(0 == m_zStream.avail_in);  // all input will be used
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
					//				if(!in.read(reinterpret_cast<char *>(&inBuffer[0]), ChunkSize)) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to read from input stream\n";
					return {};
				}

				m_zStream.avail_in = static_cast<unsigned int>(*thisRead);
				bytesRead += m_zStream.avail_in;
				m_zStream.next_in = &inBuffer[0];

				do {
					m_zStream.avail_out = outBuffer.size();
					m_zStream.next_out = &outBuffer[0];
					auto result = ::deflate(&m_zStream, Z_NO_FLUSH);
					assert(Z_STREAM_ERROR != result);
					std::copy(begin, begin + (outBuffer.size() - m_zStream.avail_out), std::back_inserter(ret));
				} while(0 == m_zStream.avail_out);
			}

			assert(0 == m_zStream.avail_in);  // all input will be used
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
				assert(Z_STREAM_ERROR != result);
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
				//				out.write(reinterpret_cast<const char *>(&outBuffer[0]), static_cast<std::streamsize>(deflatedSize));
			} while(0 == m_zStream.avail_out);

			assert(0 == m_zStream.avail_in);  // all input will be used
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
					//				if(!in.read(reinterpret_cast<char *>(&inBuffer[0]), ChunkSize)) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to read from input stream\n";
					return {};
				}

				m_zStream.avail_in = static_cast<unsigned int>(*thisRead);
				m_zStream.next_in = &inBuffer[0];
				bytesRead += m_zStream.avail_in;

				do {
					m_zStream.avail_out = outBuffer.size();
					m_zStream.next_out = &outBuffer[0];
					auto result = ::deflate(&m_zStream, Z_NO_FLUSH);
					assert(Z_STREAM_ERROR != result);
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
					//					if(!out.write(reinterpret_cast<const char *>(&outBuffer[0]), static_cast<std::streamsize>(deflatedSize))) {
					//						return {};
					//					}
				} while(0 == m_zStream.avail_out);
			}

			assert(0 == m_zStream.avail_in);
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
				assert(Z_STREAM_ERROR != result);
				std::copy(begin, begin + (outBuffer.size() - m_zStream.avail_out), std::back_inserter(ret));
			} while(0 == m_zStream.avail_out);

			assert(0 == m_zStream.avail_in);
			assert(Z_STREAM_END == result);
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
				assert(Z_STREAM_ERROR != result);
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
				//				out.write(reinterpret_cast<char *>(&outBuffer[0]), static_cast<std::streamsize>(deflatedSize));
			} while(0 == m_zStream.avail_out);

			assert(0 == m_zStream.avail_in);
			assert(Z_STREAM_END == result);
			return ret;
		}


		static inline ByteArray deflate(const ByteArray & data, int compressionLevel = -1) {
			Deflater deflater(compressionLevel);
			ByteArray ret = deflater.addData(data);
			const auto finalData = deflater.finish();
			std::copy(finalData.cbegin(), finalData.cend(), std::back_inserter(ret));
			return ret;
		}


		static inline std::optional<ByteArray> deflate(InStream & in, int compressionLevel = -1, const std::optional<int64_t> & size = {}) {
			Deflater deflater(compressionLevel);
			auto ret = deflater.addData(in, size);

			if(!ret) {
				return ret;
			}

			const auto finalData = deflater.finish();
			std::copy(finalData.cbegin(), finalData.cend(), std::back_inserter(*ret));
			return ret;
		}


		static inline std::optional<SizeType> deflateTo(OutStream & out, const ByteArray & data, int compressionLevel = -1) {
			Deflater deflater(compressionLevel);
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


		static inline std::optional<SizeType> deflateTo(OutStream & out, InStream & in, int compressionLevel = -1, const std::optional<int64_t> & size = {}) {
			Deflater deflater(compressionLevel);
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
		static constexpr const uint32_t ChunkSize = 1024;
		static constexpr const int DeflateWindowBits = 15;  // 0 - 15 produces a deflate stream
		static constexpr const int GzipWindowBits = 31;		 // 16 or greater produces a gzip stream

		static constexpr const int RawWindowBits = -15;  // -8 - -15 produces a headerless stream
		z_stream m_zStream;
	};

}  // namespace Equit

#endif  // EQUIT_DEFLATER_H
