/// \file window.h
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Declaration of the Window base class for EquitWebServer
///
/// \par Changes
/// - (2018-03) First release.

#ifndef EQUITWEBSERVER_WINDOW_H
#define EQUITWEBSERVER_WINDOW_H

#include <QMainWindow>

#include "inlinenotificationwidget.h"

class QVBoxLayout;
class QWidget;
class QString;

namespace EquitWebServer {

	using NotificationType = Equit::InlineNotificationWidget::NotificationType;

	class Window : public QMainWindow {
		Q_OBJECT
	public:
		explicit Window(QWidget * parent = nullptr);
		virtual ~Window() override;

		void showTransientInlineNotification(const QString & title, const QString & msg, NotificationType type, int timeout = 5000);

		void showTransientInlineNotification(const QString & title, const QString & msg, int timeout = 5000) {
			showTransientInlineNotification(title, msg, NotificationType::Message, timeout);
		}

		inline void showTransientInlineNotification(const QString & msg, NotificationType type, int timeout = 5000) {
			showTransientInlineNotification({}, msg, type, timeout);
		}

		inline void showTransientInlineNotification(const QString & msg, int timeout = 5000) {
			showTransientInlineNotification({}, msg, NotificationType::Message, timeout);
		}

		void showInlineNotification(const QString & title, const QString & msg, const NotificationType type = NotificationType::Message);

		inline void showInlineNotification(const QString & msg, const NotificationType type = NotificationType::Message) {
			showInlineNotification({}, msg, type);
		}

		inline QWidget * centralWidget() const {
			return m_centralWidget;
		}

		void setCentralWidget(QWidget *);

	private:
		void disposeCentralWidget();

		QVBoxLayout * m_layout;
		QWidget * m_centralWidget;
	};

}  // namespace EquitWebServer

#endif  // WINDOW_H
