/// \file iplineeditaction.h
/// \author Darren Edale
/// \version 0.9.9
/// \date February, 2018
///
/// \brief Declaration of the IpLineEditAction class for EquitWebServer
///
/// \par Changes
/// - (2018-02) First release.

#ifndef EQUITWEBSERVER_IPLINEEDITACTION_H
#define EQUITWEBSERVER_IPLINEEDITACTION_H

#include <QWidgetAction>
#include <QLineEdit>

namespace EquitWebServer {

	class IpLineEditAction : public QWidgetAction {
		Q_OBJECT

	public:
		IpLineEditAction(QObject * parent = nullptr);
		virtual ~IpLineEditAction();

		QLineEdit * lineEdit() const {
			return m_ipAddress;
		}

		QString ipAddress() const;
		void setIpAddress(const QString & addr);

	Q_SIGNALS:
		void addIpAddressClicked(const QString & addr);

	private:
		QLineEdit * m_ipAddress;
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_IPLINEEDITACTION_H
