/// \file servermimeactionsmodel.h
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Declaration of the ServerMimeActionsModel class for Anansi.
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_SERVERMIMEACTIONSMODEL_H
#define ANANSI_SERVERMIMEACTIONSMODEL_H

#include <QAbstractItemModel>
#include <QString>

#include "configuration.h"

namespace Anansi {

	class Server;

	class ServerMimeActionsModel : public QAbstractItemModel {
		Q_OBJECT

	public:
		static constexpr const int MimeTypeColumnIndex = 0;
		static constexpr const int ActionColumnIndex = 1;
		static constexpr const int CgiColumnIndex = 2;


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
		QModelIndex addMimeType(QString mimeType, WebServerAction action, const QString & cgi);
		virtual bool removeRows(int row, int count, const QModelIndex & parent) override;

	Q_SIGNALS:
		void actionChanged(const QString & mimeType, WebServerAction action);
		void cgiChanged(const QString & mimeType, const QString & cgi);

	private:
		template<int ColumnIndex>
		QModelIndex findHelper(const QString & mimeType) const;

		Server * m_server;
	};

}  // namespace Anansi

#endif  // ANANSI_SERVERMIMEACTIONSMODEL_H
