/// \file serveripconnectionpolicymodel.h
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Declaration of the ServerIpConnectionPolicyModel class for EquitWebServer
///
/// \par Changes
/// - (2018-03) First release.

#ifndef EQUITWEBSERVER_SERVERIPCONNECTIONPOLICYMODEL_H
#define EQUITWEBSERVER_SERVERIPCONNECTIONPOLICYMODEL_H

#include <QAbstractItemModel>
#include <QString>

#include "configuration.h"

namespace EquitWebServer {

	class Server;

	class ServerIpConnectionPolicyModel : public QAbstractItemModel {
		Q_OBJECT

	public:
		static constexpr const int IpAddressColumnIndex = 0;
		static constexpr const int PolicyColumnIndex = 1;

		ServerIpConnectionPolicyModel(Server * server, QObject * parent = nullptr);

		QModelIndex findIpAddress(const QString & addr) const;
		QModelIndex findIpAddressPolicy(const QString & addr) const;

		virtual QModelIndex index(int row, int column, const QModelIndex & parent = {}) const override;
		virtual QModelIndex parent(const QModelIndex & index) const override;
		virtual int rowCount(const QModelIndex & parent = {}) const override;
		virtual int columnCount(const QModelIndex & parent = {}) const override;
		virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
		virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
		virtual Qt::ItemFlags flags(const QModelIndex & index) const override;

		virtual bool setData(const QModelIndex & index, const QVariant & value, int role) override;
		QModelIndex addIpAddress(QString ipAddress, ConnectionPolicy policy);
		virtual bool removeRows(int row, int count, const QModelIndex & parent) override;

	Q_SIGNALS:
		void policyChanged(const QString & mimeType, ConnectionPolicy policy);

	private:
		template<int ColumnIndex>
		QModelIndex findHelper(const QString & addr) const;

		Server * m_server;
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_SERVERIPCONNECTIONPOLICYMODEL_H
