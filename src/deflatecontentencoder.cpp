#include "deflatecontentencoder.h"

#include <QIODevice>
#include <QByteArray>


namespace EquitWebServer {


	DeflateContentEncoder::DeflateContentEncoder(int compressionLevel)
	: ContentEncoder(),
	  m_compressionLevel(compressionLevel) {
	}


	HttpHeaders DeflateContentEncoder::headers() const {
		return {HttpHeaders::value_type{"content-encoding", "deflate"}};
	}


	bool DeflateContentEncoder::encode(QIODevice & out, const QByteArray & data) {
		if(data.isEmpty()) {
			return true;
		}

		// TODO qCompress() is simple; could use zlib directly for > efficiency
		auto deflatedData = qCompress(data, m_compressionLevel);
		uint64_t deflatedLen = static_cast<uint64_t>(deflatedData.size() - 10);

		uint64_t written = 0;
		int failCount = 0;
		auto * buffer = deflatedData.data() + 6;

		while(3 > failCount && written < deflatedLen) {
			auto thisWrite = out.write(buffer + written, static_cast<int>(deflatedLen));

			if(-1 == thisWrite) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed writing to output device\n";
				++failCount;
			}
			else {
				deflatedLen -= static_cast<uint64_t>(thisWrite);
				failCount = 0;
			}
		}

		return 0 == deflatedLen;
	}


}  // namespace EquitWebServer
