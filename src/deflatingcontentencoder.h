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


	static std::optional<int64_t> qiodeviceDeflaterRead(QIODevice & in, char * data, int64_t max) {
		auto ret = in.read(data, max);

		if(-1 == ret) {
			return {};
		}

		return ret;
	}


	static std::optional<int64_t> qiodeviceDeflaterWrite(QIODevice & out, char * data, int64_t size) {
		auto ret = out.write(data, size);

		if(-1 == ret) {
			return {};
		}

		return ret;
	}


	static bool qiodeviceDeflateStreamEnd(const QIODevice & in) {
		return in.atEnd();
	}


	using Deflater = Equit::Deflater<QByteArray, QByteArray::size_type, QIODevice, QIODevice, qiodeviceDeflaterRead, qiodeviceDeflaterWrite, qiodeviceDeflateStreamEnd>;


	template<Deflater::HeaderType headerType>
	class DeflatingContentEncoder : public ContentEncoder {
	public:
		DeflatingContentEncoder(int compressionLevel = -1)
		: ContentEncoder(),
		  m_deflater(headerType, compressionLevel) {
		}

		virtual QByteArray encode(QIODevice & dataSource, const std::optional<int64_t> & size = {}) {
			auto ret = m_deflater.addData(dataSource, size);

			if(!ret) {
				return {};
			}

			return *ret;
		}

		virtual QByteArray encode(const QByteArray & data) {
			return m_deflater.addData(data);
		}

		virtual bool encodeTo(QIODevice & out, const QByteArray & data) override {
			if(data.isEmpty()) {
				return true;
			}

			if(!m_deflater.addDataTo(out, data)) {
				return false;
			}

			return true;
		}

		virtual bool encodeTo(QIODevice & out, QIODevice & in, const std::optional<int64_t> & size = {}) {
			return !!m_deflater.addDataTo(out, in, size);
		}

		virtual bool finishEncoding(QIODevice & out) override {
			return !!m_deflater.finish(out);
			//			const auto data = m_deflater.finish();
			//			const auto * buffer = data.data();
			//			int64_t remaining = static_cast<int64_t>(data.size());

			//			while(0 < remaining) {
			//				auto written = out.write(buffer, remaining);

			//				if(-1 == written) {
			//					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed writing to output device (encoded content is likely truncated)\n";
			//					return false;
			//				}

			//				buffer += written;
			//				remaining -= written;
			//			}

			//			return 0 == remaining;
		}

	private:
		Deflater m_deflater;
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_ZLIBCONTENTENCODER_H
