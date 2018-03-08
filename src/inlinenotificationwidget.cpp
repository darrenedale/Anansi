/*
 * Copyright 2015 - 2017 Darren Edale
 *
 * This file is part of Anansi web server.
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
 * along with Anansi. If not, see <http://www.gnu.org/licenses/>.
 */

/// \file inlinenotificationwidget.cpp
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Implementation of the InlineNotificationWidget class.
///
/// \dep
/// - <iostream>
/// - <QPropertyAnimation>
///
/// \par Changes
/// - (2018-03) First release.

#include "inlinenotificationwidget.h"
#include "ui_inlinenotificationwidget.h"

#include <iostream>

#include <QPropertyAnimation>


namespace Equit {


	static QColor WarningBackground = QColor::fromHsv(60, 128, 64);
	static QColor ErrorBackground = QColor::fromHsv(0, 128, 64);


	InlineNotificationWidget::InlineNotificationWidget(const QString & title, const QString & msg, QWidget * parent)
	: InlineNotificationWidget(NotificationType::Message, msg, parent) {
		m_ui->title->setText(title);
	}


	InlineNotificationWidget::InlineNotificationWidget(const QString & msg, QWidget * parent)
	: InlineNotificationWidget(NotificationType::Message, msg, parent) {
	}


	InlineNotificationWidget::InlineNotificationWidget(QWidget * parent)
	: InlineNotificationWidget(NotificationType::Message, {}, parent) {
	}


	InlineNotificationWidget::InlineNotificationWidget(const InlineNotificationWidget::NotificationType & type, const QString & msg, QWidget * parent)
	: QWidget(parent),
	  m_type(type),
	  m_ui(std::make_unique<Ui::InlineNotificationWidget>()),
	  m_showAnim(std::make_unique<QPropertyAnimation>(this, "maximumHeight")),
	  m_hideAnim(std::make_unique<QPropertyAnimation>(this, "maximumHeight")) {
		m_ui->setupUi(this);
		m_ui->message->setText(msg);

		QFont titleFont = m_ui->title->font();
		titleFont.setPointSizeF(titleFont.pointSizeF() * 1.2);
		m_ui->title->setFont(titleFont);

		if(NotificationType::Warning == type) {
			m_ui->notificationFrame->setStyleSheet("background-color: " + WarningBackground.name() + ";");
		}
		else if(NotificationType::Error == type) {
			m_ui->notificationFrame->setStyleSheet("background-color: " + ErrorBackground.name() + ";");
		}
		//		else {
		//			m_ui->notificationFrame->setStyleSheet("background-color: " + MessageBackground.name() + ";");
		//		}

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
