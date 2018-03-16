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

/// \file mediatypeicons.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of functions to handle media type icons for Anansi.
///
/// \dep
/// - <algorithm>
/// - <QByteArray>
/// - <QIcon>
/// - <QString>
/// - <QStringBuilder>
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_MEDIATYPEICONS_H
#define ANANSI_MEDIATYPEICONS_H

#include <algorithm>

#include <QIcon>
#include <QByteArray>
#include <QString>
#include <QStringBuilder>

namespace Anansi {


	namespace MediaTypeIconFlags {
		static constexpr const int Default = 0x00;
		static constexpr const int NoThemeIcon = 0x01;
		static constexpr const int NoGenericIcon = 0x02;
	}  // namespace MediaTypeIconFlags


	namespace MediaTypeIcons {
		static const QString ResourcePath = QStringLiteral(":/icons/mediatypes/");
		static constexpr const int DefaultSize = 32;
	}  // namespace MediaTypeIcons


	template<class InStringType, class OutStringType = InStringType>
	OutStringType mediaTypeIconName(const InStringType & mediaType) {
		OutStringType ret;
		ret.reserve(mediaType.size() + 1);

		std::transform(mediaType.cbegin(), mediaType.cend(), std::back_inserter(ret), [](const auto & ch) {
			if('/' == ch) {
				return decltype(ch)('-');
			}

			return ch;
		});

		return ret;
	}


	template<int flags = MediaTypeIconFlags::Default>
	static QIcon mediaTypeIcon(const QString & mediaType) {
		auto iconName = mediaTypeIconName(mediaType);
		QIcon icon;

		if constexpr(flags & MediaTypeIconFlags::NoThemeIcon) {
			icon = QIcon(MediaTypeIcons::ResourcePath % iconName);
		}
		else {
			icon = QIcon::fromTheme(iconName, QIcon(MediaTypeIcons::ResourcePath % iconName));
		}

		if constexpr(!(flags & MediaTypeIconFlags::NoGenericIcon)) {
			if(icon.isNull()) {
				auto pos = mediaType.indexOf('/');

				if(-1 != pos) {
					iconName = mediaType.left(mediaType.indexOf('/')) % QStringLiteral("-x-generic");

					if constexpr(flags & MediaTypeIconFlags::NoThemeIcon) {
						icon = QIcon(MediaTypeIcons::ResourcePath % iconName);
					}
					else {
						icon = QIcon::fromTheme(iconName, QIcon(MediaTypeIcons::ResourcePath % iconName));
					}
				}
			}
		}

		return icon;
	}


	QByteArray mediaTypeIconUri(const QString &, int = MediaTypeIcons::DefaultSize);


}  // namespace Anansi

#endif  // ANANSI_MEDIATYPEICONS_H
