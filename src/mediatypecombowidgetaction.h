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

/// \file mediatypecombowidgetaction.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of the MediaTypeComboWidgetAction class for Anansi.
///
/// \dep
/// - <QWidgetAction>
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_MEDIATYPECOMBOWIDGETACTION_H
#define ANANSI_MEDIATYPECOMBOWIDGETACTION_H

#include <QWidgetAction>

#include <vector>

namespace Anansi {

	class MediaTypeCombo;

	class MediaTypeComboWidgetAction : public QWidgetAction {
		Q_OBJECT

	public:
		MediaTypeComboWidgetAction(QObject * parent = nullptr);

		MediaTypeCombo * mediaTypeCombo() const noexcept {
			return m_combo;
		}

		void setMediaTypes(std::vector<QString> mediaTypes);
		void addMediaType(const QString & mediaType);

	Q_SIGNALS:
		void addMediaTypeClicked(const QString &);

	private:
		MediaTypeCombo * m_combo;
	};

}  // namespace Anansi

#endif  // ANANSI_MEDIATYPECOMBOWIDGETACTION_H
