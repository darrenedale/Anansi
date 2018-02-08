#ifndef EQUITWEBSERVER_SERVERCONFIGWIDGET_H
#define EQUITWEBSERVER_SERVERCONFIGWIDGET_H

#include <memory>

#include <QWidget>


namespace EquitWebServer {

	namespace Ui {
		class ServerConfigWidget;
	}

	class ServerConfigWidget : public QWidget {
		Q_OBJECT

	public:
		explicit ServerConfigWidget(QWidget * parent = nullptr);
		virtual ~ServerConfigWidget();

		QString documentRoot() const;
		QString listenIpAddress() const;
		quint16 listenPort() const;

	public Q_SLOTS:
		void chooseDocumentRoot();
		void setDocumentRoot(const QString &);
		void setListenIpAddress(const QString &);
		void setListenPort(uint16_t);

	Q_SIGNALS:
		void documentRootChanged(const QString &);
		void listenIpAddressChanged(const QString &);
		void listenPortChanged(uint16_t);

	private:
		void repopulateLocalAddresses();

		std::unique_ptr<Ui::ServerConfigWidget> m_ui;
	};


}  // namespace EquitWebServer
#endif  // EQUITWEBSERVER_SERVERCONFIGWIDGET_H
