#ifndef EQUITWEBSERVER_GZIPCONTENTENCODER_H
#define EQUITWEBSERVER_GZIPCONTENTENCODER_H

#include <QFile>

#include "deflatecontentencoder.h"
#include "crc32.h"

namespace EquitWebServer {

	using Equit::Crc32;

	class GzipContentEncoder : public DeflateContentEncoder {
	public:
		GzipContentEncoder(int compressionLevel = -1);

		virtual HttpHeaders headers() const override;
		virtual bool startEncoding(QIODevice & out) override;
		virtual bool encodeTo(QIODevice & out, const QByteArray & data) override;
		virtual bool finishEncoding(QIODevice & out) override;

	private:
		Crc32 m_crc32;
		uint64_t m_uncompressedSize;
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_GZIPCONTENTENCODER_H
