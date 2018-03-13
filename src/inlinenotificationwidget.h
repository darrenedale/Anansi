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

/// \file inlinenotificationwidget.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of the InlineNotificationWidget class for Equit.
///
/// \dep
/// - <memory>
/// - <QWidget>
///
/// \par Changes
/// - (2018-03) First release.

#ifndef EQ_INLINENOTIFICATIONWIDGET_H
#define EQ_INLINENOTIFICATIONWIDGET_H

#include <memory>

#include <QWidget>

class QPropertyAnimation;

namespace Equit {

	namespace Ui {
		class InlineNotificationWidget;
	}

	class InlineNotificationWidget
	: public QWidget {
		Q_OBJECT

	public:
		enum class NotificationType {
			Message = 0,
			Warning,
			Error,
			Question,  // for future use
		};

		InlineNotificationWidget(const QString &, const QString &, QWidget * = nullptr);
		explicit InlineNotificationWidget(const QString &, QWidget * = nullptr);
		explicit InlineNotificationWidget(QWidget * = nullptr);
		InlineNotificationWidget(const NotificationType &, const QString & = {}, QWidget * = nullptr);
		~InlineNotificationWidget() override;

		inline const NotificationType & type() const noexcept {
			return m_type;
		}

		QString message() const;
		QString title() const;
		bool closeButtonIsVisible() const;

		void setTitle(const QString & title);
		void setMessage(const QString & msg);

		void showCloseButton();
		void hideCloseButton();

		void setVisible(bool vis) override;

	Q_SIGNALS:
		void closed();

	private:
		NotificationType m_type;
		std::unique_ptr<Ui::InlineNotificationWidget> m_ui;
		std::unique_ptr<QPropertyAnimation> m_showAnim;
		std::unique_ptr<QPropertyAnimation> m_hideAnim;
	};

}  // namespace Equit

#endif  // EQ_INLINENOTIFICATIONWIDGET_H
