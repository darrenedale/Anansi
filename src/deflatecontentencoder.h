#ifndef EQUITWEBSERVER_DEFLATECONTENTENCODER_H
#define EQUITWEBSERVER_DEFLATECONTENTENCODER_H

#include "contentencoder.h"
#include "deflater.h"

namespace EquitWebServer {

	class DeflateContentEncoder : public ContentEncoder {
	public:
		DeflateContentEncoder(int compressionLevel = -1);

		virtual HttpHeaders headers() const override;
		virtual bool encodeTo(QIODevice &, const QByteArray & data) override;
		virtual bool finishEncoding(QIODevice &) override;

	private:
		Equit::Deflater m_deflater;
		int m_compressionLevel;
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_DEFLATECONTENTENCODER_H
