#ifndef EQUITWEBSERVER_GZIPCONTENTENCODER_H
#define EQUITWEBSERVER_GZIPCONTENTENCODER_H

#include <QFile>

#include "contentencoder.h"
#include "crc32.h"

namespace EquitWebServer {

	using Equit::Crc32;

	class GzipContentEncoder : public ContentEncoder {
	public:
		GzipContentEncoder(int compressionLevel = -1);

		virtual HttpHeaders headers() const override;
		virtual bool startEncoding(QIODevice & out) override;
		virtual bool encode(QIODevice & out, const QByteArray & data) override;
		virtual bool finishEncoding(QIODevice & out) override;

	private:
		int m_compressionLevel;
		int m_compressedSize;
		uint64_t m_uncompressedSize;
		Crc32 m_crc32;
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_GZIPCONTENTENCODER_H
