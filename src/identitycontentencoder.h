#ifndef EQUITWEBSERVER_IDENTITYCONTENTENCODER_H
#define EQUITWEBSERVER_IDENTITYCONTENTENCODER_H

#include "contentencoder.h"

namespace EquitWebServer {

	class IdentityContentEncoder : public ContentEncoder {
	public:
		IdentityContentEncoder() = default;

		virtual bool encode(QIODevice &, const QByteArray & data) override;
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_IDENTITYCONTENTENCODER_H
