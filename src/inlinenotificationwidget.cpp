#include "inlinenotificationwidget.h"
#include "ui_inlinenotificationwidget.h"

#include <iostream>

#include <QPropertyAnimation>


namespace Equit {


	static const QColor MessageBackground = Qt::transparent;
	static const QColor WarningBackground = QColor(200, 200, 150);
	static const QColor ErrorBackground = QColor(200, 150, 150);


	InlineNotificationWidget::InlineNotificationWidget(const QString & title, const QString & msg, QWidget * parent)
	: InlineNotificationWidget(NotificationType::Message, parent) {
		m_ui->title->setText(title);
		m_ui->message->setText(msg);
	}


	InlineNotificationWidget::InlineNotificationWidget(const QString & msg, QWidget * parent)
	: InlineNotificationWidget(NotificationType::Message, parent) {
		m_ui->message->setText(msg);
	}


	InlineNotificationWidget::InlineNotificationWidget(QWidget * parent)
	: InlineNotificationWidget(NotificationType::Message, parent) {
	}

	InlineNotificationWidget::InlineNotificationWidget(const InlineNotificationWidget::NotificationType & type, QWidget * parent)
	: QWidget(parent),
	  m_type(type),
	  m_ui(std::make_unique<Ui::InlineNotificationWidget>()),
	  m_showAnim(std::make_unique<QPropertyAnimation>(this, "maximumHeight")),
	  m_hideAnim(std::make_unique<QPropertyAnimation>(this, "maximumHeight")) {
		m_ui->setupUi(this);

		QFont titleFont = m_ui->title->font();
		titleFont.setPointSizeF(titleFont.pointSizeF() * 1.2);
		m_ui->title->setFont(titleFont);

		if(NotificationType::Warning == type) {
			m_ui->notificationFrame->setStyleSheet("background-color: " + WarningBackground.name() + ";");
		}
		else if(NotificationType::Error == type) {
			m_ui->notificationFrame->setStyleSheet("background-color: " + ErrorBackground.name() + ";");
		}
		else {
			m_ui->notificationFrame->setStyleSheet("background-color: " + MessageBackground.name() + ";");
		}

		m_showAnim->setStartValue(0);
		m_showAnim->setEndValue(50);
		m_hideAnim->setEndValue(0);
		m_showAnim->setDuration(300);
		m_hideAnim->setDuration(300);
		m_showAnim->setEasingCurve(QEasingCurve::InOutQuad);
		m_hideAnim->setEasingCurve(QEasingCurve::InOutQuad);

		connect(m_hideAnim.get(), &QAbstractAnimation::finished, [this]() {
			QWidget::setVisible(false);
			Q_EMIT closed();
		});

		connect(m_ui->close, &QPushButton::clicked, this, &InlineNotificationWidget::hide);
	}


	InlineNotificationWidget::~InlineNotificationWidget() = default;


	QString InlineNotificationWidget::message() const {
		return m_ui->message->text();
	}


	QString InlineNotificationWidget::title() const {
		return m_ui->title->text();
	}

	bool InlineNotificationWidget::closeButtonIsVisible() const {
		return m_ui->close->isVisible();
	}


	void InlineNotificationWidget::setMessage(const QString & msg) {
		m_ui->message->setText(msg);
	}


	void InlineNotificationWidget::setTitle(const QString & title) {
		m_ui->title->setText(title);
	}


	void InlineNotificationWidget::showCloseButton() {
		m_ui->close->show();
	}


	void InlineNotificationWidget::hideCloseButton() {
		m_ui->close->hide();
	}


	void InlineNotificationWidget::setVisible(bool vis) {
		if(vis) {
			if(QAbstractAnimation::Stopped != m_showAnim->state()) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: show animation already running\n";
				return;
			}


			if(isVisible()) {
				return;
			}

			m_ui->title->setVisible(!m_ui->title->text().isEmpty());
			title().isEmpty();

			setMaximumHeight(QWIDGETSIZE_MAX);
			adjustSize();
			m_showAnim->setEndValue(height());
			setMaximumHeight(0);
			QWidget::setVisible(true);
			m_showAnim->start();
		}
		else {
			if(QAbstractAnimation::Stopped != m_hideAnim->state()) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: hide animation already running\n";
				return;
			}

			if(isHidden()) {
				return;
			}

			m_hideAnim->start();
		}
	}


}  // namespace Equit
