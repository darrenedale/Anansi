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

/// \file mediatypecombo.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of the MediaTypeCombo class for Anansi.
///
/// \dep
/// - <vector>
/// - <QString>
/// - <QComboBox>
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_MEDIATYPECOMBO_H
#define ANANSI_MEDIATYPECOMBO_H

#include <vector>

#include <QString>
#include <QComboBox>

namespace Anansi {

	class MediaTypeCombo : public QComboBox {
		Q_OBJECT

	public:
		explicit MediaTypeCombo(bool allowCustomTypes, QWidget * parent = nullptr);
		explicit MediaTypeCombo(QWidget * parent = nullptr);

		void insertItem() = delete;
		void insertItems() = delete;
		void addItem() = delete;
		void addItems() = delete;
		void removeItem() = delete;

		std::vector<QString> availableMediaTypes() const;
		QString currentMediaType() const;

		inline bool customMediaTypesAllowed() const {
			return isEditable();
		}

		bool hasMediaType(const QString & mediaType) const;

	public Q_SLOTS:
		inline void setCustomMediaTypesAllowed(bool allowed) {
			setEditable(allowed);
		}

		bool addMediaType(const QString & mediaType);
		void removeMediaType(const QString & mediaType);
		void setCurrentMediaType(const QString & mediaType);

	Q_SIGNALS:
		void mediaTypeAdded(const QString & mediaType);
		void mediaTypeRemoved(const QString & mediaType);
		void currentMediaTypeChanged(const QString & mediaType);
	};

}  // namespace Anansi

#endif  // ANANSI_MEDIATYPECOMBO_H
