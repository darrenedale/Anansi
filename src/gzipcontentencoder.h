#ifndef GZIPCONTENTENCODER_H
#define GZIPCONTENTENCODER_H

#include "contentencoder.h"

namespace EquitWebServer {

	class GzipContentEncoder : public ContentEncoder {
	public:
		GzipContentEncoder(int compressionLevel = -1);

		virtual bool beginSending(QTcpSocket & socket) override;
		virtual bool sendData(QTcpSocket & socket, const QByteArray & data) override;
		virtual bool finishSending(QTcpSocket & socket) override;

	private:
		int m_compressionLevel;
	};

}  // namespace EquitWebServer

#endif  // GZIPCONTENTENCODER_H
