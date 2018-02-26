#include "mainwindowstatusbar.h"

#include "counterlabel.h"


namespace EquitWebServer {


	MainWindowStatusBar::MainWindowStatusBar(QWidget * parent)
	: QStatusBar(parent),
	  m_received(new CounterLabel(tr("Requests received: %1"), 0, this)),
	  m_accepted(new CounterLabel(tr("Requests accepted: %1"), 0, this)),
	  m_rejected(new CounterLabel(tr("Requests rejected: %1"), 0, this)) {
		addPermanentWidget(m_received);
		addPermanentWidget(m_accepted);
		addPermanentWidget(m_rejected);
	}


	MainWindowStatusBar::~MainWindowStatusBar() = default;


	void MainWindowStatusBar::resetReceived() {
		m_received->reset();
	}


	void MainWindowStatusBar::resetAccepted() {
		m_accepted->reset();
	}


	void MainWindowStatusBar::resetRejected() {
		m_rejected->reset();
	}


	void MainWindowStatusBar::incrementReceived() {
		m_received->increment();
	}


	void MainWindowStatusBar::incrementAccepted() {
		m_accepted->increment();
	}


	void MainWindowStatusBar::incrementRejected() {
		m_rejected->increment();
	}


	void MainWindowStatusBar::resetAllCounters() {
		m_received->reset();
		m_accepted->reset();
		m_rejected->reset();
	}


}  // namespace EquitWebServer
