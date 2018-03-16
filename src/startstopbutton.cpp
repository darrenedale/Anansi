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

/// \file startstopbutton.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation of the StartStopButton class.
///
/// \dep
/// - startstopbutton.h
///
/// \par Changes
/// - (2018-03) First release.

#include "startstopbutton.h"


namespace Anansi {


	static constexpr const StartStopButton::State DefaultState = StartStopButton::State::Start;
	static constexpr const char * DefaultStartText = "Start";
	static constexpr const char * DefaultStopText = "Stop";
	static const QString DefaultStartIcon = QStringLiteral("media-playback-start");
	static const QString DefaultStopIcon = QStringLiteral("media-playback-stop");


	StartStopButton::StartStopButton(StartStopButton::State state, QWidget * parent)
	: QPushButton(parent),
	  m_state(state),
	  m_autoToggle(false) {
		refresh();

		connect(this, &QPushButton::clicked, [this]() {
			switch(m_state) {
				case State::Start:
					Q_EMIT startClicked();
					break;

				case State::Stop:
					Q_EMIT stopClicked();
					break;
			}

			if(m_autoToggle) {
				toggleState();
			}
		});
	}


	StartStopButton::StartStopButton(QWidget * parent)
	: StartStopButton(DefaultState, parent) {
	}


	QString StartStopButton::startText() const {
		return m_startText.value_or(tr(DefaultStartText));
	}


	void StartStopButton::setStartText(const QString & text) {
		m_startText = text;

		if(State::Start == m_state) {
			refresh();
		}
	}


	void StartStopButton::useDefaultStartText() {
		m_startText = {};

		if(State::Start == m_state) {
			refresh();
		}
	}


	QString StartStopButton::stopText() const {
		return m_stopText.value_or(tr(DefaultStopText));
	}


	void StartStopButton::setStopText(const QString & text) {
		m_stopText = text;

		if(State::Stop == m_state) {
			refresh();
		}
	}


	void StartStopButton::useDefaultStopText() {
		m_stopText = {};

		if(State::Stop == m_state) {
			refresh();
		}
	}


	QIcon StartStopButton::startIcon() const {
		return m_startIcon.value_or(QIcon::fromTheme(DefaultStartIcon));
	}


	void StartStopButton::setStartIcon(const QIcon & icon) {
		m_startIcon = icon;

		if(State::Start == m_state) {
			refresh();
		}
	}


	void StartStopButton::useDefaultStartIcon() {
		m_startIcon = {};

		if(State::Start == m_state) {
			refresh();
		}
	}


	QIcon StartStopButton::stopIcon() const {
		return m_stopIcon.value_or(QIcon::fromTheme(DefaultStopIcon));
	}


	void StartStopButton::setStopIcon(const QIcon & icon) {
		m_stopIcon = icon;

		if(State::Stop == m_state) {
			refresh();
		}
	}


	void StartStopButton::useDefaultStopIcon() {
		m_stopIcon = {};

		if(State::Stop == m_state) {
			refresh();
		}
	}


	void StartStopButton::setState(StartStopButton::State state) {
		if(state != m_state) {
			m_state = state;
			refresh();
		}
	}

	void StartStopButton::toggleState() {
		switch(m_state) {
			case State::Start:
				m_state = State::Stop;
				break;

			case State::Stop:
				m_state = State::Start;
				break;
		}

		refresh();
	}


	void StartStopButton::refresh() {
		switch(m_state) {
			case State::Start:
				setText(startText());
				setIcon(startIcon());
				break;

			case State::Stop:
				setText(stopText());
				setIcon(stopIcon());
				break;
		}
	}

}  // namespace Anansi
