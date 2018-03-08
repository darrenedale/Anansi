#ifndef ANANSI_DEFLATECONTENTENCODER_H
#define ANANSI_DEFLATECONTENTENCODER_H

#include "zlibcontentencoder.h"

namespace Anansi {

	class DeflateContentEncoder : public ZLibContentEncoder<ZLibDeflaterHeaderType::Deflate> {
	public:
		virtual HttpHeaders headers() const override;
	};

}  // namespace Anansi

#endif  // ANANSI_DEFLATECONTENTENCODER_H
