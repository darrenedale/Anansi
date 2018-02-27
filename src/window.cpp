#include "window.h"

#include <iostream>

#include <QLayout>
#include <QTimer>

namespace EquitWebServer {

	/// \class Window
	/// \brief Base class for top-level windows in the EquitWebServer application.
	///
	/// The base class extends QMainWindow with an inline notifications feature that
	/// enables child widgets to show window-level notifications. All they need to
	/// do to be able to do this is to qobject_cast() window() to
	/// EquitWebServer::Window * and if it's not `nullptr`, call showTransientInlineNotification()
	/// or showInlineNotification().


	using Equit::InlineNotificationWidget;


	Window::Window(QWidget * parent)
	: QMainWindow(parent),
	  m_layout(new QVBoxLayout),
	  m_centralWidget(nullptr) {
		QWidget * container = new QWidget;
		container->setLayout(m_layout);
		QMainWindow::setCentralWidget(container);
	}


	Window::~Window() {
		disposeCentralWidget();
	};


	void Window::showTransientInlineNotification(const QString & title, const QString & msg, int timeout) {
		if(100 > timeout) {
			timeout = 100;
		}

		auto * notification = new InlineNotificationWidget(title, msg, this);
		m_layout->insertWidget(0, notification, 0);
		connect(notification, &InlineNotificationWidget::closed, notification, &QObject::deleteLater);
		QTimer::singleShot(timeout, notification, &QWidget::hide);
	}


	void Window::showInlineNotification(const QString & title, const QString & msg, const NotificationType type) {
		auto * notification = new InlineNotificationWidget(type, this);
		notification->setTitle(title);
		notification->setMessage(msg);
		m_layout->insertWidget(0, notification, 0);
		connect(notification, &InlineNotificationWidget::closed, notification, &QObject::deleteLater);
	}


	void Window::setCentralWidget(QWidget * widget) {
		disposeCentralWidget();
		m_centralWidget = widget;

		if(widget) {
			m_layout->addWidget(m_centralWidget);
		}
	}


	void Window::disposeCentralWidget() {
		if(m_centralWidget && m_centralWidget->parent() == m_layout) {
			m_layout->removeWidget(m_centralWidget);
			delete m_centralWidget;
		}

		m_centralWidget = nullptr;
	}

}  // namespace EquitWebServer
