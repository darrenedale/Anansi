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
/// \dep
/// - <memory>
/// - <QStatusBar>
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_MAINWINDOWSTATUSBAR_H
#define ANANSI_MAINWINDOWSTATUSBAR_H

#include <memory>

#include <QStatusBar>

namespace Equit {
	class CounterLabel;
}

namespace Anansi {
	class MainWindowStatusBar : public QStatusBar {
	public:
		explicit MainWindowStatusBar(QWidget * parent = nullptr);

	public Q_SLOTS:
		void resetReceived();
		void resetAccepted();
		void resetRejected();
		void resetAllCounters();
		void incrementReceived();
		void incrementAccepted();
		void incrementRejected();

	private:
		std::unique_ptr<Equit::CounterLabel> m_received;
		std::unique_ptr<Equit::CounterLabel> m_accepted;
		std::unique_ptr<Equit::CounterLabel> m_rejected;
	};
}  // namespace Anansi

#endif  // ANANSI_MAINWINDOWSTATUSBAR_H
