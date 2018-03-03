/// \file deflatecontentencoder.h
/// \author Darren Edale
/// \version 0.9.9
/// \date February, 2018
///
/// \brief Definition of the DeflateContentEncoder class for Equit.
///
/// \todo This and Deflater need lots of attention - the implementation
/// is VERY suboptimal
///
/// \par Changes
/// - (2018-02) First release.

#include "deflatecontentencoder.h"

#include <iostream>

#include <QIODevice>
#include <QByteArray>


namespace EquitWebServer {


	DeflateContentEncoder::DeflateContentEncoder(int compressionLevel)
	: ContentEncoder(),
	  m_deflater(compressionLevel),
	  m_compressionLevel(compressionLevel) {
	}


	HttpHeaders DeflateContentEncoder::headers() const {
		return {HttpHeaders::value_type{"content-encoding", "deflate"}};
	}


	bool DeflateContentEncoder::encodeTo(QIODevice & out, const QByteArray & data) {
		if(data.isEmpty()) {
			return true;
		}

		const auto compressed = m_deflater.addData(data.toStdString());
		auto res = out.write(compressed.data(), static_cast<int>(compressed.size()));
		return -1 != res && compressed.size() == static_cast<std::size_t>(res);

		//		// TODO qCompress() is simple; could use zlib directly for > efficiency
		//		auto deflatedData = qCompress(data, m_compressionLevel);
		//		int64_t size = deflatedData.size() - 10;
		//		int failCount = 0;
		//		auto * buffer = deflatedData.data() + 6;

		//		while(3 > failCount && 0 < size) {
		//			auto written = out.write(buffer, size);

		//			if(-1 == written) {
		//				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed writing to output device\n";
		//				++failCount;
		//			}
		//			else {
		//				size -= written;
		//				buffer += written;
		//				failCount = 0;
		//			}
		//		}

		//		return 0 == size;
	}


	bool DeflateContentEncoder::finishEncoding(QIODevice & out) {
		const auto data = m_deflater.finish();
		const auto * buffer = data.data();
		qint64 remaining = static_cast<qint64>(data.size());
		int failCount = 0;

		while(3 > failCount && 0 < remaining) {
			auto written = out.write(buffer, remaining);

			if(-1 == written) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed writing to output device\n";
				++failCount;
			}
			else {
				failCount = 0;
				remaining -= written;
				buffer += written;
			}
		}

		return 0 == remaining;
	}


}  // namespace EquitWebServer
