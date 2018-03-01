#ifndef EQUITWEBSERVER_CONTENTENCODER_H
#define EQUITWEBSERVER_CONTENTENCODER_H

#include <iostream>

#include "types.h"

class QIODevice;
class QByteArray;

namespace EquitWebServer {

	class RequestHandler;

	class ContentEncoder {
	public:
		ContentEncoder() = default;
		virtual ~ContentEncoder() = default;

		virtual HttpHeaders headers() const {
			std::cout << "content encoder sends no headers\n";
			return {};
		}

		virtual bool startEncoding(QIODevice &) {
			std::cout << "content encoder has no special startup\n";
			return true;
		}

		virtual bool encode(QIODevice &, const QByteArray & data) = 0;

		virtual bool finishEncoding(QIODevice &) {
			std::cout << "content encoder has no special cleanup\n";
			return true;
		}
	};

}  // namespace EquitWebServer

#endif  // CONTENTENCODER_H
