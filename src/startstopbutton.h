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

#ifndef ANANSI_STARTSTOPBUTTON_H
#define ANANSI_STARTSTOPBUTTON_H

#include <optional>

#include <QPushButton>
#include <QString>
#include <QIcon>

namespace Anansi {

	class StartStopButton : public QPushButton {
		Q_OBJECT
	public:
		enum class State {
			Start = 0,
			Stop,
		};

		StartStopButton(QWidget * parent = nullptr);
		StartStopButton(State state, QWidget * parent = nullptr);

		inline State currentState() const {
			return m_state;
		}

		inline bool autoToggleState() const {
			return m_autoToggleState;
		}

		inline void setAutoToggleState(bool toggle) {
			m_autoToggleState = toggle;
		}

		inline bool hasCustomStartText() const {
			return static_cast<bool>(m_startText);
		}

		QString startText() const;
		void setStartText(const QString &);
		inline void useDefaultStartText();

		inline bool hasCustomStopText() const {
			return static_cast<bool>(m_stopText);
		}

		QString stopText() const;
		void setStopText(const QString &);
		void useDefaultStopText();

		inline bool hasCustomStartIcon() const {
			return static_cast<bool>(m_startIcon);
		}

		QIcon startIcon() const;
		void setStartIcon(const QIcon &);
		inline void useDefaultStartIcon();

		inline bool hasCustomStopIcon() const {
			return static_cast<bool>(m_stopIcon);
		}

		QIcon stopIcon() const;
		void setStopIcon(const QIcon &);
		void useDefaultStopIcon();

	public Q_SLOTS:
		void setState(State);
		void toggleState();

	Q_SIGNALS:
		void startClicked();
		void stopClicked();

	protected:
		void refresh();

	private:
		State m_state;
		bool m_autoToggleState;
		std::optional<QString> m_startText;
		std::optional<QString> m_stopText;
		std::optional<QIcon> m_startIcon;
		std::optional<QIcon> m_stopIcon;
	};

}  // namespace Anansi

#endif  // ANANSI_STARTSTOPBUTTON_H
