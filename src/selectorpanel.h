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

/// \file selectorpanel.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of the SelectorPanel class for Anansi.
///
/// \dep
/// - <QListWidget>
/// - <QListWidgetItem>
/// - <QIcon>
/// - <QString>
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_SELECTORPANEL_H
#define ANANSI_SELECTORPANEL_H

#include <QListWidget>
#include <QListWidgetItem>
#include <QIcon>
#include <QString>

class QShowEvent;

namespace Anansi {

	class SelectorPanel : public QListWidget {
	public:
		explicit SelectorPanel(QWidget * = nullptr);

		void addItem(QListWidgetItem *);

		inline void addItem(const QIcon & icon, const QString & label) {
			addItem(new QListWidgetItem(icon, label));
		}

		void insertItem(int, QListWidgetItem *);

		inline void insertItem(int row, const QIcon & icon, const QString & label) {
			insertItem(row, new QListWidgetItem(icon, label));
		}

		void addItems() = delete;
		void insertItems() = delete;

	protected:
		void showEvent(QShowEvent *) override;

	private:
		void recalculateSize();
	};

}  // namespace Anansi

#endif  // ANANSI_SELECTORPANEL_H
