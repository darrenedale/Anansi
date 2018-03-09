#include "startstopbutton.h"

namespace Anansi {


	static constexpr const StartStopButton::State DefaultState = StartStopButton::State::Start;
	static constexpr const char * DefaultStartText = "Start";
	static constexpr const char * DefaultStopText = "Stop";
	static constexpr const char * DefaultStartIcon = "media-playback-start";
	static constexpr const char * DefaultStopIcon = "media-playback-stop";


	StartStopButton::StartStopButton(QWidget * parent)
	: StartStopButton(DefaultState, parent) {
	}


	StartStopButton::StartStopButton(StartStopButton::State state, QWidget * parent)
	: QPushButton(parent),
	  m_state(state),
	  m_autoToggleState(false) {
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

			if(m_autoToggleState) {
				toggleState();
			}
		});
	}


	QString StartStopButton::startText() const {
		return m_startText.value_or(tr("Start"));
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
