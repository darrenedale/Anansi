/*
 * Copyright 2015 - 2018 Darren Edale
 *
 * This file is part of Anansi web server.
 *
 * Anansi is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Anansi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Anansi. If not, see <http://www.gnu.org/licenses/>.
 */

/// \file mimeicons.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation of functions to hande MIME type icons for Anansi..
///
/// \dep
/// - <iostream>
/// - <QByteArray>
/// - <QIcon>
/// - <QFile>
/// - <QStringBuilder>
/// - <QBuffer>
///
/// \par Changes
/// - (2018-03) First release.

#include "mimeicons.h"

#include <iostream>

#include <QByteArray>
#include <QIcon>
#include <QFile>
#include <QStringBuilder>
#include <QBuffer>

namespace Anansi {


	QString mimeIconUri(const QString & mimeType, int size) {
		auto icon = mimeIcon(mimeType);

		if(icon.isNull()) {
			return {};
		}

		QByteArray data;
		QBuffer pngBuffer(&data);

		if(!pngBuffer.open(QIODevice::WriteOnly)) {
			return {};
		}

		icon.pixmap(size).save(&pngBuffer, "PNG");
		pngBuffer.close();

		return QStringLiteral("data:image/png;base64,") % data.toBase64();
	}


}  // namespace Anansi
