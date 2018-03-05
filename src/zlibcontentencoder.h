/// \file zlibcontentencoder.h
/// \author Darren Edale
/// \version 0.9.9
/// \date March, 2018
///
/// \brief Definition of the ZLibContentEncoder template class for EquitWebServer.
///
/// This is a template base class for content encoders that use zlib (via the
/// Deflater class) to compress (deflate) content for transfer to the user agent.
/// It is templated on the type of header that the Deflater class will use.
/// Instantiating with HeaderType::Deflate creates a content encoder suitable for
/// use with the "deflate" content encoding; instantiating with HeaderType::Gzip
/// creates a content encoder suitable for use with the "gzip" content encoding.
///
/// It is strongly recommended that this template is not instantiated directly;
/// rather, it should be used via an inheriting class that reimplements the
/// headers() method to provide the appropriate headers for the response to the
/// user agent. The two template instantiations DeflateContentEncoder and
/// GzipContentEncoder do this.
///
/// \todo Needs review as the implementation is only quickly drafted and is almost
/// certainly suboptimal.
///
/// \par Changes
/// - (2018-03) First release.

#ifndef EQUITWEBSERVER_ZLIBCONTENTENCODER_H
#define EQUITWEBSERVER_ZLIBCONTENTENCODER_H

#include <QByteArray>
#include <QIODevice>

#include "contentencoder.h"
#include "deflater.h"

namespace EquitWebServer {

	using Equit::Deflater;

	template<Deflater::HeaderType headerType>
	class ZLibContentEncoder : public ContentEncoder {
	public:
		ZLibContentEncoder(int compressionLevel = -1)
		: ContentEncoder(),
		  m_deflater(headerType, compressionLevel) {
		}

		virtual bool encodeTo(QIODevice & out, const QByteArray & data) override {
			if(data.isEmpty()) {
				return true;
			}

			const auto compressed = m_deflater.addData(data.toStdString());
			auto res = out.write(compressed.data(), static_cast<int>(compressed.size()));
			return -1 != res && compressed.size() == static_cast<std::size_t>(res);
		}

		virtual bool finishEncoding(QIODevice & out) override {
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

	private:
		Deflater m_deflater;
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_ZLIBCONTENTENCODER_H
