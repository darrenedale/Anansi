/// \file inlinenotificationwidget.h
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Declaration of the InlineNotificationWidget class for Equit.
///
/// \par Changes
/// - (2018-03) First release.

#ifndef EQUIT_INLINENOTIFICATIONWIDGET_H
#define EQUIT_INLINENOTIFICATIONWIDGET_H

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
			Warning = 1,
			Error = 2,
			Question = 3,  // for future use
		};

		InlineNotificationWidget(const QString & title, const QString & msg, QWidget * parent = nullptr);
		explicit InlineNotificationWidget(const QString & msg, QWidget * parent = nullptr);
		explicit InlineNotificationWidget(QWidget * parent = nullptr);
		InlineNotificationWidget(const NotificationType & type, QWidget * parent = nullptr);
		~InlineNotificationWidget() override;

		inline const NotificationType & type() const {
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

#endif  // EQUIT_INLINENOTIFICATIONWIDGET_H
