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

/// \file webserveractioncombo.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of the WebServerActionCombo class for Anansi.
///
/// \dep
/// - <QComboBox>
/// - types.h
///
/// NEXTRELEASE investigate whether this can be templated for ConnectionPolicy,
/// WebServerAction, DirectoryListingSortOrderCombo, ...
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_WEBSERVERACTIONCOMBO_H
#define ANANSI_WEBSERVERACTIONCOMBO_H

#include <QComboBox>

#include "types.h"

namespace Anansi {

	class WebServerActionCombo : public QComboBox {
		Q_OBJECT

	public:
		explicit WebServerActionCombo(QWidget * parent = nullptr);
		WebServerActionCombo(const WebServerActionCombo &) = delete;
		WebServerActionCombo(WebServerActionCombo &&) = delete;
		void operator=(const WebServerActionCombo &) = delete;
		void operator=(WebServerActionCombo &&) = delete;

		void addItem() = delete;
		void addItems() = delete;
		void insertItem() = delete;
		void insertItems() = delete;
		void removeItem() = delete;

		WebServerAction webServerAction() const;

	public Q_SLOTS:
		void setWebServerAction(WebServerAction action);

	Q_SIGNALS:
		void webServerActionChanged(WebServerAction) const;
	};

}  // namespace Anansi

#endif  // ANANSI_WEBSERVERACTIONCOMBO_H
