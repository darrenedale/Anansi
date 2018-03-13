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
/// \brief Declaration of functions to handle MIME type icons for Anansi.
///
/// \dep
/// - <algorithm>
/// - <QIcon>
/// - <QString>
/// - <QStringBuilder>
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_MIMEICONS_H
#define ANANSI_MIMEICONS_H

#include <algorithm>
#include <QIcon>
#include <QByteArray>
#include <QString>
#include <QStringBuilder>

namespace Anansi {


	namespace MimeIconFlags {
		static constexpr const int Default = 0x00;
		static constexpr const int NoThemeIcon = 0x01;
		static constexpr const int NoGenericIcon = 0x02;
	}  // namespace MimeIconFlags


	namespace MimeIcons {
		static const QString ResourcePath = QStringLiteral(":/icons/mime/");
		static constexpr const int DefaultSize = 32;
	}  // namespace MimeIcons


	template<class T>
	T mimeIconName(const T & mime) {
		T ret;
		ret.reserve(mime.size() + 1);

		std::transform(mime.cbegin(), mime.cend(), std::back_inserter(ret), [](const auto & ch) {
			if('/' == ch) {
				return decltype(ch)('-');
			}

			return ch;
		});

		return ret;
	}


	template<int flags = MimeIconFlags::Default>
	static QIcon mimeIcon(const QString & mime) {
		auto iconName = mimeIconName(mime);
		QIcon icon;

		if constexpr(flags & MimeIconFlags::NoThemeIcon) {
			icon = QIcon(MimeIcons::ResourcePath % iconName);
		}
		else {
			icon = QIcon::fromTheme(iconName, QIcon(MimeIcons::ResourcePath % iconName));
		}

		if constexpr(!(flags & MimeIconFlags::NoGenericIcon)) {
			if(icon.isNull()) {
				auto pos = mime.indexOf('/');

				if(-1 != pos) {
					iconName = mime.left(mime.indexOf('/')) % QStringLiteral("-x-generic");

					if constexpr(flags & MimeIconFlags::NoThemeIcon) {
						icon = QIcon(MimeIcons::ResourcePath % iconName);
					}
					else {
						icon = QIcon::fromTheme(iconName, QIcon(MimeIcons::ResourcePath % iconName));
					}
				}
			}
		}

		return icon;
	}


	QByteArray mimeIconUri(const QString &, int = MimeIcons::DefaultSize);


}  // namespace Anansi

#endif  // ANANSI_MIMEICONS_H
