#ifndef ANANSI_GZIPCONTENTENCODER_H
#define ANANSI_GZIPCONTENTENCODER_H

#include "zlibcontentencoder.h"

namespace Anansi {

	class GzipContentEncoder : public ZLibContentEncoder<ZLibDeflaterHeaderType::Gzip> {
	public:
		virtual HttpHeaders headers() const override;
	};

}  // namespace Anansi

#endif  // ANANSI_GZIPCONTENTENCODER_H
