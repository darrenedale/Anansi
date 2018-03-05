/*
 * Copyright 2015 - 2017 Darren Edale
 *
 * This file is part of EquitWebServer.
 *
 * Qonvince is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Qonvince is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EquitWebServer. If not, see <http://www.gnu.org/licenses/>.
 */

/// \file accesslogwidget.cpp
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Implementation of the AccessLogWidget class.
///
/// \dep
/// - <iostream>
/// - <QString>
///
/// \par Changes
/// - (2018-03) First release.

#include "accesslogwidget.h"

#include <iostream>
#include <QString>
#include <QTreeWidgetItem>


Q_DECLARE_METATYPE(EquitWebServer::ConnectionPolicy);
Q_DECLARE_METATYPE(EquitWebServer::WebServerAction);


namespace EquitWebServer {


	AccessLogWidget::AccessLogWidget(QWidget * parent)
	: QTreeWidget(parent) {
		QTreeWidgetItem * accessLogHeader = new QTreeWidgetItem;
		accessLogHeader->setText(0, tr("Remote IP"));
		accessLogHeader->setText(1, tr("Remote Port"));
		accessLogHeader->setText(2, tr("Resource Requested"));
		accessLogHeader->setText(3, tr("Response/Action"));
		// QTreeWidget takes ownership
		setHeaderItem(accessLogHeader);
		setRootIsDecorated(false);
	}


	void AccessLogWidget::addPolicyEntry(const QString & addr, uint16_t port, ConnectionPolicy policy) {
		addTopLevelItem(new AccessLogTreeItem(addr, port, policy));
	}


	void AccessLogWidget::addActionEntry(const QString & addr, uint16_t port, QString resource, WebServerAction action) {
		addTopLevelItem(new AccessLogTreeItem(addr, port, resource, action));
	}


}  // namespace EquitWebServer
