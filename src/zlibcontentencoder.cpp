#include "zlibcontentencoder.h"

namespace Anansi {

	namespace Detail {
		std::optional<int64_t> qiodeviceDeflaterRead(QIODevice & in, char * data, int64_t max) {
			auto ret = in.read(data, max);

			if(-1 == ret) {
				return {};
			}

			return ret;
		}


		std::optional<int64_t> qiodeviceDeflaterWrite(QIODevice & out, char * data, int64_t size) {
			auto ret = out.write(data, size);

			if(-1 == ret) {
				return {};
			}

			return ret;
		}


		bool qiodeviceDeflateStreamEnd(const QIODevice & in) {
			return in.atEnd();
		}
	}  // namespace Detail

}  // namespace Anansi
