#ifndef EQUITWEBSERVER_SERVERMIMEACTIONSMODEL_H
#define EQUITWEBSERVER_SERVERMIMEACTIONSMODEL_H

#include <QAbstractItemModel>
#include <QString>

#include "configuration.h"

namespace EquitWebServer {

	class Server;

	class ServerMimeActionsModel : public QAbstractItemModel {
		Q_OBJECT

	public:
		ServerMimeActionsModel(Server * server, QObject * parent = nullptr);

		QModelIndex findMimeType(const QString & mimeType) const;
		QModelIndex findMimeTypeAction(const QString & mimeType) const;
		QModelIndex findMimeTypeCgi(const QString & mimeType) const;

		virtual QModelIndex index(int row, int column, const QModelIndex & parent = {}) const override;
		virtual QModelIndex parent(const QModelIndex & index) const override;
		virtual int rowCount(const QModelIndex & parent = {}) const override;
		virtual int columnCount(const QModelIndex & parent = {}) const override;
		virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
		virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
		virtual Qt::ItemFlags flags(const QModelIndex & index) const override;

		virtual bool setData(const QModelIndex & index, const QVariant & value, int role) override;
		QModelIndex addMimeType(QString mimeType, Configuration::WebServerAction action, const QString & cgi);
		virtual bool removeRows(int row, int count, const QModelIndex & parent) override;

	Q_SIGNALS:
		void actionChanged(const QString & mimeType, Configuration::WebServerAction action);
		void cgiChanged(const QString & mimeType, const QString & cgi);

	private:
		template<int ColumnIndex>
		QModelIndex findHelper(const QString & mimeType) const;

		Server * m_server;
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_SERVERMIMEACTIONSMODEL_H
