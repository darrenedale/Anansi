/// \file mainwindowstatusbar.h
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Declaration of the MainWindowStatusBar class for Anansi.
///
/// \par Changes
/// - (2018-03) First release.

#ifndef MAINWINDOWSTATUSBAR_H
#define MAINWINDOWSTATUSBAR_H

#include <QStatusBar>

namespace Anansi {

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
}  // namespace Anansi

#endif  // MAINWINDOWSTATUSBAR_H
