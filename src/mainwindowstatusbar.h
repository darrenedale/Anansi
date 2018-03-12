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

/// \file mainwindowstatusbar.h
/// \author Darren Edale
/// \version 1.0.0
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
