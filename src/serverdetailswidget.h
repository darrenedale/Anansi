/// \file serverdetailswidget.h
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Declaration of the ServerDetailsWidget class for Anansi.
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_SERVERCONFIGWIDGET_H
#define ANANSI_SERVERCONFIGWIDGET_H

#include <memory>

#include <QWidget>


namespace Anansi {

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
		QString administratorEmail() const;
		QString cgiBin() const;

	public Q_SLOTS:
		void chooseDocumentRoot();
		void setDocumentRoot(const QString &);
		void setListenAddress(const QString &);
		void setListenPort(uint16_t);
		void setAdministratorEmail(const QString &);
		void chooseCgiBin();
		void setCgiBin(const QString &);

	Q_SIGNALS:
		void documentRootChanged(const QString &);
		void listenIpAddressChanged(const QString &);
		void listenPortChanged(uint16_t);
		void administratorEmailChanged(const QString &);
		void cgiBinChanged(const QString &);

	private:
		void repopulateLocalAddresses();

		std::unique_ptr<Ui::ServerDetailsWidget> m_ui;
	};


}  // namespace Anansi
#endif  // ANANSI_SERVERCONFIGWIDGET_H
