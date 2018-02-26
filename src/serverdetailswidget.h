#ifndef EQUITWEBSERVER_SERVERCONFIGWIDGET_H
#define EQUITWEBSERVER_SERVERCONFIGWIDGET_H

#include <memory>

#include <QWidget>


namespace EquitWebServer {

	namespace Ui {
		class ServerDetailsWidget;
	}

	class ServerDetailsWidget : public QWidget {
		Q_OBJECT

	public:
		explicit ServerDetailsWidget(QWidget * parent = nullptr);
		virtual ~ServerDetailsWidget();

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

		std::unique_ptr<Ui::ServerDetailsWidget> m_ui;
	};


}  // namespace EquitWebServer
#endif  // EQUITWEBSERVER_SERVERCONFIGWIDGET_H
