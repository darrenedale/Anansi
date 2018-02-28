#ifndef EQUITWEBSERVER_CONTENTENCODER_H
#define EQUITWEBSERVER_CONTENTENCODER_H

class QTcpSocket;
class QByteArray;

namespace EquitWebServer {

	class ContentEncoder {
	public:
		ContentEncoder() = default;
		virtual ~ContentEncoder() = default;

		virtual bool beginSending(QTcpSocket &) {
			return true;
		}

		virtual bool sendData(QTcpSocket &, const QByteArray & data) = 0;

		virtual bool finishSending(QTcpSocket &) {
			return true;
		}
	};

}  // namespace EquitWebServer

#endif  // CONTENTENCODER_H
