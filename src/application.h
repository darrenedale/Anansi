/// \file application.h
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Declaration of the Application class for Anansi..
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
		virtual ~Application();

		static void ensureUserConfigDir();

		static inline Application * instance() {
			return qobject_cast<Application *>(QApplication::instance());
		}

	private:
		std::unique_ptr<MainWindow> m_mainWindow;
	};

}  // namespace Anansi

#endif  // ANANSI_APPLICATION_H
