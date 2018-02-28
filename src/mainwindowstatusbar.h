/// \file mainwindowstatusbar.h
/// \author Darren Edale
/// \version 0.9.9
/// \date February, 2018
///
/// \brief Declaration of the MainWindowStatusBar class for EquitWebServer
///
/// \par Changes
/// - (2018-02) First release.

#ifndef MAINWINDOWSTATUSBAR_H
#define MAINWINDOWSTATUSBAR_H

#include <QStatusBar>

namespace EquitWebServer {

	class CounterLabel;

	class MainWindowStatusBar : public QStatusBar {
	public:
		explicit MainWindowStatusBar(QWidget * parent = nullptr);

		~MainWindowStatusBar();

	public Q_SLOTS:
		void resetReceived();
		void resetAccepted();
		void resetRejected();
		void resetAllCounters();
		void incrementReceived();
		void incrementAccepted();
		void incrementRejected();

	private:
		CounterLabel * m_received;
		CounterLabel * m_accepted;
		CounterLabel * m_rejected;
	};
}  // namespace EquitWebServer

#endif  // MAINWINDOWSTATUSBAR_H
