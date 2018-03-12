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

/// \file mimeicons.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of functions to handle MIME type icons for Anansi..
///
/// \dep
/// - <algorithm>
/// - <QString>
/// - <QStringBuilder>
/// - <QIcon>
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_MIMEICONS_H
#define ANANSI_MIMEICONS_H

#include <algorithm>
#include <QIcon>
#include <QString>
#include <QStringBuilder>

namespace Anansi {


	static const QString MimeIconResourcePath = QStringLiteral(":/icons/mime/");
	static constexpr const int NoThemeIcon = 0x01;
	static constexpr const int NoGenericIcon = 0x02;


	template<class T>
	T mimeIconName(const T & mimeType) {
		T ret;
		ret.reserve(mimeType.size() + 1);

		std::transform(mimeType.cbegin(), mimeType.cend(), std::back_inserter(ret), [](const auto & ch) {
			if('/' == ch) {
				return decltype(ch)('-');
			}

			return ch;
		});

		return ret;
	}


	template<int flags = 0>
	static QIcon mimeIcon(const QString & mimeType) {
		QString iconName = mimeIconName(mimeType);
		QIcon icon;

		if constexpr(flags & NoThemeIcon) {
			icon = QIcon(MimeIconResourcePath % iconName);
		}
		else {
			icon = QIcon::fromTheme(iconName, QIcon(MimeIconResourcePath % iconName));
		}

		if constexpr(!(flags & NoGenericIcon)) {
			if(icon.isNull()) {
				auto pos = mimeType.indexOf('/');

				if(-1 != pos) {
					iconName = mimeType.left(mimeType.indexOf('/')) % QStringLiteral("-x-generic");

					if constexpr(flags & NoThemeIcon) {
						icon = QIcon(MimeIconResourcePath % iconName);
					}
					else {
						icon = QIcon::fromTheme(iconName, QIcon(MimeIconResourcePath % iconName));
					}
				}
			}
		}

		return icon;
	}


	QString mimeIconUri(const QString & mimeType, int size = 32);


}  // namespace Anansi

#endif  // ANANSI_MIMEICONS_H
