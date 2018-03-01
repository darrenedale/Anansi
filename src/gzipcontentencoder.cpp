#include "gzipcontentencoder.h"

#include <QIODevice>
#include <QByteArray>
#include <QtEndian>


namespace EquitWebServer {


	static constexpr const char * GzipHeader = "\x1f\x8b\x08\x00\x00\x00\x00\x00\x00\x03";


	GzipContentEncoder::GzipContentEncoder(int compressionLevel)
	: ContentEncoder(),
	  m_compressionLevel(compressionLevel),
	  m_compressedSize(0),
	  m_uncompressedSize(0) {
	}


	HttpHeaders GzipContentEncoder::headers() const {
		return {HttpHeaders::value_type{"content-encoding", "gzip"}};
	}


	bool GzipContentEncoder::startEncoding(QIODevice & out) {
		out.write(GzipHeader, 10);
		m_crc32.reset();
		m_uncompressedSize = 0;
		m_compressedSize = 0;
		return true;
	}


	bool GzipContentEncoder::encode(QIODevice & out, const QByteArray & data) {
		if(data.isEmpty()) {
			return true;
		}

		m_crc32.addData(data);
		m_uncompressedSize += static_cast<unsigned int>(data.size());

		// TODO qCompress() is simple; could use zlib directly for > efficiency
		auto gzData = qCompress(data, m_compressionLevel);
		uint64_t compressedLen = static_cast<uint64_t>(gzData.size() - 10);
		m_compressedSize += compressedLen;

		uint64_t written = 0;
		int failCount = 0;
		auto * buffer = gzData.data() + 6;

		while(3 > failCount && written < compressedLen) {
			auto thisWrite = out.write(buffer + written, static_cast<int>(compressedLen));

			if(-1 == thisWrite) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed writing to output device\n";
				++failCount;
			}
			else {
				compressedLen -= static_cast<uint64_t>(thisWrite);
				failCount = 0;
			}
		}

		return 0 == compressedLen;
	}


	bool GzipContentEncoder::finishEncoding(QIODevice & out) {
		uint32_t crc32 = qToLittleEndian(m_crc32.intResult());
		uint32_t iSize = static_cast<uint32_t>(0xffffffff & qToLittleEndian(m_uncompressedSize));
		out.write(reinterpret_cast<const char *>(&crc32), 4);
		out.write(reinterpret_cast<const char *>(&iSize), 4);
		return true;
	}


}  // namespace EquitWebServer
