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

/// \file accesslogtreeitem.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of the AccessLogTreeItem class for Anansi.
///
/// \dep
/// - <cstdint>
/// - <QTreeWidgetItem>
/// - types.h
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_ACCESSLOGTREEITEM_H
#define ANANSI_ACCESSLOGTREEITEM_H

#include <cstdint>

#include <QTreeWidgetItem>

#include "types.h"

class QString;
class QDateTime;

namespace Anansi {

	class AccessLogTreeItem : public QTreeWidgetItem {
	public:
		explicit AccessLogTreeItem(const QDateTime & timestamp, const QString & addr, uint16_t port, const QString & resource, WebServerAction action);
		explicit AccessLogTreeItem(const QDateTime & timestamp, const QString & addr, uint16_t port, ConnectionPolicy policy);

		void setIpAddress(const QString & addr);
		void setPort(uint16_t port);
		void setResource(const QString & resource);
		void setAction(WebServerAction action);
		void setConnectionPolicy(ConnectionPolicy policy);
	};

}  // namespace Anansi

#endif  // ANANSI_ACCESSLOGTREEITEM_H
