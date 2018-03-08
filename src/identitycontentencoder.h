#ifndef ANANSI_IDENTITYCONTENTENCODER_H
#define ANANSI_IDENTITYCONTENTENCODER_H

#include "contentencoder.h"

namespace Anansi {

	class IdentityContentEncoder : public ContentEncoder {
	public:
		IdentityContentEncoder() = default;

		virtual bool encodeTo(QIODevice &, const QByteArray & data) override;
	};

}  // namespace Anansi

#endif  // ANANSI_IDENTITYCONTENTENCODER_H
