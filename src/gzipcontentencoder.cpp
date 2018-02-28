#include "gzipcontentencoder.h"

#include <QTcpSocket>
#include <QByteArray>


namespace EquitWebServer {


	GzipContentEncoder::GzipContentEncoder(int compressionLevel)
	: ContentEncoder(),
	  m_compressionLevel(compressionLevel) {
	}


	bool GzipContentEncoder::sendData(QTcpSocket & socket, const QByteArray & data) {
		socket.write(data);
	}


}  // namespace EquitWebServer
