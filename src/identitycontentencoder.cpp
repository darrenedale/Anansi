#include "identitycontentencoder.h"

#include <QIODevice>
#include <QByteArray>

#include <iostream>


namespace EquitWebServer {


	bool IdentityContentEncoder::encodeTo(QIODevice & out, const QByteArray & data) {
		uint64_t written = 0;
		uint64_t length = static_cast<uint64_t>(data.size());
		int failCount = 0;

		while(3 > failCount && written < length) {
			auto thisWrite = out.write(data.data() + written, static_cast<int>(length));

			if(-1 == thisWrite) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed writing to socket\n";
				++failCount;
			}
			else {
				length -= static_cast<uint64_t>(thisWrite);
				failCount = 0;
			}
		}

		return 0 == length;
	}


}  // namespace EquitWebServer
