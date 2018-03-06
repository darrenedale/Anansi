#ifndef EQUITWEBSERVER_GZIPCONTENTENCODER_H
#define EQUITWEBSERVER_GZIPCONTENTENCODER_H

#include "deflatingcontentencoder.h"

namespace EquitWebServer {

	class GzipContentEncoder : public DeflatingContentEncoder<Deflater::HeaderType::Gzip> {
	public:
		virtual HttpHeaders headers() const override;
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_GZIPCONTENTENCODER_H
