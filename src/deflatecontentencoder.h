#ifndef EQUITWEBSERVER_DEFLATECONTENTENCODER_H
#define EQUITWEBSERVER_DEFLATECONTENTENCODER_H

#include "deflatingcontentencoder.h"

namespace EquitWebServer {

	class DeflateContentEncoder : public DeflatingContentEncoder<Deflater::HeaderType::Deflate> {
	public:
		virtual HttpHeaders headers() const override;
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_DEFLATECONTENTENCODER_H
