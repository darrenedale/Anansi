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

/// \file application.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of the Application class for Anansi..
///
/// \dep
/// - <memory>
/// - <QApplication>
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_APPLICATION_H
#define ANANSI_APPLICATION_H

#include <memory>

#include <QApplication>

#define awsApp Anansi::Application::instance()

namespace Anansi {

	class MainWindow;

	class Application : public QApplication {
		Q_OBJECT

	public:
		Application(int & argc, char ** argv);
		~Application() override;

		static void ensureUserConfigDir();

		static inline Application * instance() {
			return qobject_cast<Application *>(QApplication::instance());
		}

	private:
		std::unique_ptr<MainWindow> m_mainWindow;
	};

}  // namespace Anansi

#endif  // ANANSI_APPLICATION_H
