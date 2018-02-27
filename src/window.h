#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>

#include "inlinenotificationwidget.h"

namespace EquitWebServer {

	using NotificationType = Equit::InlineNotificationWidget::NotificationType;

	class Window : public QMainWindow {
		Q_OBJECT
	public:
		explicit Window(QWidget * parent = nullptr);
		virtual ~Window() override;

		void showTransientInlineNotification(const QString & title, const QString & msg, int timeout = 5000);

		inline void showTransientInlineNotification(const QString & msg, int timeout = 5000) {
			showTransientInlineNotification({}, msg, timeout);
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
