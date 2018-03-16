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

/// \file configurationwidget.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Definition of the ConfigurationWidget class for Anansi.
///
/// \dep
/// - <memory>
/// - <QWidget>
/// - types.h
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_CONFIGURATIONWIDGET_H
#define ANANSI_CONFIGURATIONWIDGET_H

#include <memory>

#include <QWidget>

#include "types.h"

class QString;

namespace Anansi {

	class Server;

	namespace Ui {
		class ConfigurationWidget;
	}

	/// \class ConfigurationWidget
	class ConfigurationWidget : public QWidget {
		Q_OBJECT

	public:
		explicit ConfigurationWidget(QWidget * parent = nullptr);
		explicit ConfigurationWidget(Server * server, QWidget * parent = nullptr);
		virtual ~ConfigurationWidget() override;

		void setServer(Server * server);
		void readConfiguration();

	public Q_SLOTS:
		void chooseDocumentRoot();

		void bindToLocalhost();
		void bindToHostAddress();
		void setListenAddress(const QString & addr);

		void clearIpConnectionPolicies();
		void setLiberalDefaultConnectionPolicy();
		void setRestrictiveDefaultConnectionPolicy();
		void setDefaultConnectionPolicy(ConnectionPolicy policy);

		void setDefaultMediaType(const QString & mediaType);
		void setDefaultAction(WebServerAction action);

		void clearAllFileExtensionMediaTypes();
		void clearAllActions();

	private:
		// observed only
		Server * m_server;
		std::unique_ptr<Ui::ConfigurationWidget> m_ui;
	};

}  // namespace Anansi

#endif  //	ANANSI_CONFIGURATIONWIDGET_H
