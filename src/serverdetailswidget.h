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
/// \version 0.9.9
/// \date March 2018
///
/// \brief Declaration of the ServerDetailsWidget class for Anansi.
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_SERVERCONFIGWIDGET_H
#define ANANSI_SERVERCONFIGWIDGET_H

#include <memory>

#include <QWidget>


namespace Anansi {

	namespace Ui {
		class ServerDetailsWidget;
	}

	class ServerDetailsWidget : public QWidget {
		Q_OBJECT

	public:
		explicit ServerDetailsWidget(QWidget * parent = nullptr);
		virtual ~ServerDetailsWidget();

		QString documentRoot() const;
		QString listenIpAddress() const;
		quint16 listenPort() const;
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
		void documentRootChanged(const QString &);
		void listenIpAddressChanged(const QString &);
		void listenPortChanged(uint16_t);
		void administratorEmailChanged(const QString &);
		void cgiBinChanged(const QString &);

	private:
		void repopulateLocalAddresses();

		std::unique_ptr<Ui::ServerDetailsWidget> m_ui;
	};


}  // namespace Anansi
#endif  // ANANSI_SERVERCONFIGWIDGET_H
