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

/// \file mimecombowidgetaction.h
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Declaration of the Configuration class for Anansi.
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_MIMECOMBOWIDGETACTION_H
#define ANANSI_MIMECOMBOWIDGETACTION_H

#include <QWidgetAction>

namespace Anansi {

	class MimeCombo;

	class MimeComboWidgetAction : public QWidgetAction {
		Q_OBJECT

	public:
		MimeComboWidgetAction(QObject * parent = nullptr);
		virtual ~MimeComboWidgetAction();

		MimeCombo * mimeCombo() const {
			return m_combo;
		}

		void setMimeTypes(std::vector<QString> mimeTypes);
		void addMimeType(const QString & mimeType);

	Q_SIGNALS:
		void addMimeTypeClicked(const QString & mimeType);

	private:
		MimeCombo * m_combo;
	};

}  // namespace Anansi

#endif  // ANANSI_MIMECOMBOWIDGETACTION_H
