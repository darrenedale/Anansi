#include "deflatecontentencoder.h"

#include <iostream>

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


	/// TODO does not work correctly when data is provided in more than one call
	bool DeflateContentEncoder::encodeTo(QIODevice & out, const QByteArray & data) {
		if(data.isEmpty()) {
			return true;
		}

		// TODO qCompress() is simple; could use zlib directly for > efficiency
		auto deflatedData = qCompress(data, m_compressionLevel);
		int64_t deflatedLen = deflatedData.size() - 10;
		int64_t written = 0;
		int failCount = 0;
		auto * buffer = deflatedData.data() + 6;

		while(3 > failCount && written < deflatedLen) {
			auto thisWrite = out.write(buffer + written, deflatedLen);

			if(-1 == thisWrite) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed writing to output device\n";
				++failCount;
			}
			else {
				deflatedLen -= thisWrite;
				failCount = 0;
			}
		}

		return 0 == deflatedLen;
	}


}  // namespace EquitWebServer
