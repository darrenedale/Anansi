#ifndef EQUITWEBSERVER_DEFLATECONTENTENCODER_H
#define EQUITWEBSERVER_DEFLATECONTENTENCODER_H

#include "zlibcontentencoder.h"

namespace EquitWebServer {

	class DeflateContentEncoder : public ZLibContentEncoder<Deflater::HeaderType::Deflate> {
	public:
		virtual HttpHeaders headers() const override;
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_DEFLATECONTENTENCODER_H
