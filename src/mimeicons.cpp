#include "mimeicons.h"

#include <iostream>

#include <QByteArray>
#include <QIcon>
#include <QFile>
#include <QStringBuilder>
#include <QBuffer>

namespace EquitWebServer {


	QString mimeIconUri(const QString & mimeType, int size) {
		auto icon = mimeIcon(mimeType);

		if(icon.isNull()) {
			return {};
		}

		// TODO cache
		QByteArray data;
		QBuffer pngBuffer(&data);

		if(!pngBuffer.open(QIODevice::WriteOnly)) {
			return {};
		}

		icon.pixmap(size).save(&pngBuffer, "PNG");
		pngBuffer.close();

		return QStringLiteral("data:image/png;base64,") % data.toBase64();
	}


}  // namespace EquitWebServer
