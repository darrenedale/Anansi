#ifndef EQUITWEBSERVER_SERVERFILEASSOCIATIONSMODEL_H
#define EQUITWEBSERVER_SERVERFILEASSOCIATIONSMODEL_H

#include <QAbstractItemModel>

namespace EquitWebServer {

	class Server;

	class ServerFileAssociationsModel : public QAbstractItemModel {
	public:
		ServerFileAssociationsModel(Server * server, QObject * parent = nullptr);

		virtual QModelIndex index(int row, int column, const QModelIndex & parent = {}) const override;
		virtual QModelIndex parent(const QModelIndex & index) const override;
		virtual int rowCount(const QModelIndex & parent = {}) const override;
		virtual int columnCount(const QModelIndex & parent = {}) const override;
		virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;

	private:
		Server * m_server;
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_SERVERFILEASSOCIATIONSMODEL_H
