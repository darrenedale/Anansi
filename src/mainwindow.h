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

/// \file mainwindow.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Definition of the MainWindow class for Anansi.
///
/// \dep
/// - <memory>
/// - <vector>
/// - windowbase.h
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_MAINWINDOW_H
#define ANANSI_MAINWINDOW_H

#include <memory>
#include <vector>

#include "windowbase.h"

class QString;
class QAction;
class QActionGroup;

namespace Anansi {

	class Server;
	class MainWindowStatusBar;

	namespace Ui {
		class MainWindow;
	}

	class MainWindow : public WindowBase {
		Q_OBJECT

	public:
		explicit MainWindow(QWidget * parent = nullptr);
		explicit MainWindow(std::unique_ptr<Server> = nullptr, QWidget * parent = nullptr);
		~MainWindow() override;

		void setServer(std::unique_ptr<Server> server);

	public Q_SLOTS:
		bool startServer();
		bool stopServer();
		void about();

		void saveConfiguration();
		void saveConfigurationAsDefault();
		void loadConfiguration();
		void loadConfiguration(const QString & fileName);

		void incrementRequestReceivedCount();
		void incrementRequestAcceptedCount();
		void incrementRequestRejectedCount();
		void resetRequestReceivedCount();
		void resetRequestAcceptedCount();
		void resetRequestRejectedCount();
		void resetAllRequestCounts();

		void setStatusMessage(const QString & msg);
		MainWindowStatusBar * statusBar() const;

	protected:
		void showEvent(QShowEvent *);
		void closeEvent(QCloseEvent *);

	private:
		QAction * addRecentConfiguration(const QString & path);
		void readRecentConfigurations();
		void saveRecentConfigurations();

		std::unique_ptr<Server> m_server;
		std::unique_ptr<Ui::MainWindow> m_ui;
		std::vector<std::unique_ptr<QAction>> m_recentConfigActions;
		std::unique_ptr<QActionGroup> m_recentConfigActionGroup;
	};

}  // namespace Anansi

#endif
