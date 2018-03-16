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

/// \file mediatypeicons.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation of functions to hande media type icons for Anansi.
///
/// \dep
/// - mediatypeicons.h
/// - <QBuffer>
///
/// \par Changes
/// - (2018-03) First release.

#include "mediatypeicons.h"

#include <QBuffer>


namespace Anansi {


	QByteArray mediaTypeIconUri(const QString & mediaType, int size) {
		auto icon = mediaTypeIcon(mediaType);

		if(icon.isNull()) {
			return {};
		}

		QByteArray pngData;
		QBuffer pngBuffer(&pngData);

		if(!pngBuffer.open(QIODevice::WriteOnly)) {
			return {};
		}

		icon.pixmap(size).save(&pngBuffer, "PNG");
		pngBuffer.close();

		return QByteArrayLiteral("data:image/png;base64,") % pngData.toBase64();
	}


}  // namespace Anansi
