/// \file mimeicons.h
/// \author Darren Edale
/// \version 0.9.9
/// \date February, 2018
///
/// \brief Functions to handle MIME type icons for EquitWebServer.
///
/// \par Changes
/// - (2018-02) First release.

#ifndef EQUITWEBSERVER_MIMEICONS_H
#define EQUITWEBSERVER_MIMEICONS_H

#include <QIcon>
#include <QString>
#include <QStringBuilder>

namespace EquitWebServer {


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


}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_MIMEICONS_H
