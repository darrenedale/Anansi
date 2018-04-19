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

/// \file serverdetailswidget.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of the ServerDetailsWidget class for Anansi.
///
/// \dep
/// - <memory>
/// - <QWidget>
/// - <QString>
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_SERVERDETAILSWIDGET_H
#define ANANSI_SERVERDETAILSWIDGET_H

#include <memory>

#include <QWidget>
#include <QString>

namespace Anansi {

	class Server;

	namespace Ui {
		class ServerDetailsWidget;
	}

	class ServerDetailsWidget : public QWidget {
		Q_OBJECT

	public:
		explicit ServerDetailsWidget(QWidget * parent = nullptr);
		explicit ServerDetailsWidget(Server * server, QWidget * parent = nullptr);
		~ServerDetailsWidget() override;

		void setServer(Server * server);

		QString documentRoot() const;
		QString listenIpAddress() const;
		uint16_t listenPort() const;
		QString administratorEmail() const;
		QString cgiBin() const;

	public Q_SLOTS:
		void chooseDocumentRoot();
		void setDocumentRoot(const QString &);
		void setListenAddress(const QString &);
		void setListenPort(uint16_t);
		void setAdministratorEmail(const QString &);
		void chooseCgiBin();
		void setCgiBin(const QString &);

	Q_SIGNALS:
		void documentRootChanged(const QString &) const;
		void listenIpAddressChanged(const QString &) const;
		void listenPortChanged(uint16_t) const;
		void administratorEmailChanged(const QString &) const;
		void cgiBinChanged(const QString &) const;

	private:
		void repopulateLocalAddresses();
		void clearStatuses();

		std::unique_ptr<Ui::ServerDetailsWidget> m_ui;
		Server * m_server;
	};

}  // namespace Anansi

#endif  // ANANSI_SERVERDETAILSWIDGET_H
