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
