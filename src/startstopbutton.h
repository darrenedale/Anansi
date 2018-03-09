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
