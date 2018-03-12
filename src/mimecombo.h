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

/// \file mimecombo.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of the MimeCombo class for Anansi.
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_MIMECOMBO_H
#define ANANSI_MIMECOMBO_H

#include <vector>

#include <QString>
#include <QComboBox>

#include "configuration.h"

namespace Anansi {

	class MimeCombo : public QComboBox {
		Q_OBJECT

	public:
		explicit MimeCombo(QWidget * parent = nullptr);
		explicit MimeCombo(bool allowCustom, QWidget * parent = nullptr);
		virtual ~MimeCombo() = default;

		void insertItem() = delete;
		void addItem() = delete;
		void removeItem() = delete;

		std::vector<QString> availableMimeTypes() const;
		QString currentMimeType() const;

		inline bool customMimeTypesAllowed() const {
			return isEditable();
		}

		bool hasMimeType(const QString & mime) const;

	public Q_SLOTS:
		void setCustomMimeTypesAllowed(bool allowed) {
			setEditable(allowed);
		}

		bool addMimeType(const QString & mime);
		void removeMimeType(const QString & mime);
		void setCurrentMimeType(const QString & mime);

	Q_SIGNALS:
		void mimeTypeAdded(const QString &);
		void mimeTypeRemoved(const QString &);
		void currentMimeTypeChanged(const QString &);
	};

}  // namespace Anansi

#endif  // ANANSI_MIMECOMBO_H
