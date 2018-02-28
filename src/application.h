/// \file application.h
/// \author Darren Edale
/// \version 0.9.9
/// \date February, 2018
///
/// \brief Declaration of the Application class for EquitWebServer.
///
/// \par Changes
/// - (2018-02) First release.

#ifndef EQUITWEBSERVER_APPLICATION_H
#define EQUITWEBSERVER_APPLICATION_H

#include <memory>

#include <QApplication>

#define wsApp (EquitWebServer::Application::instance())

namespace EquitWebServer {

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

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_APPLICATION_H
