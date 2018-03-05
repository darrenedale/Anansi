/*
 * Copyright 2015 - 2017 Darren Edale
 *
 * This file is part of EquitWebServer.
 *
 * Qonvince is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Qonvince is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EquitWebServer. If not, see <http://www.gnu.org/licenses/>.
 */

/// \file mainwindowstatusbar.cpp
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Implementation of the MainWindowStatusBar class.
///
/// \dep
/// - counterlabel.h
///
/// \par Changes
/// - (2018-03) First release.

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
