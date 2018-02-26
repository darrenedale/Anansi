#include "servermimeactionsmodel.h"

#include <iostream>

#include "numerics.h"
#include "server.h"
#include "mimeicons.h"


Q_DECLARE_METATYPE(EquitWebServer::Configuration::WebServerAction)


namespace EquitWebServer {


	template<int ColumnIndex>
	QModelIndex ServerMimeActionsModel::findHelper(const QString & mimeType) const {
		const auto mimeTypes = m_server->configuration().registeredMimeTypes();
		const auto & begin = mimeTypes.cbegin();
		const auto & end = mimeTypes.cend();
		const auto mimeTypeIt = std::find(begin, end, mimeType);

		if(end == mimeTypeIt) {
			return {};
		}

		return createIndex(static_cast<int>(std::distance(begin, mimeTypeIt)), ColumnIndex);
	}


	ServerMimeActionsModel::ServerMimeActionsModel(Server * server, QObject * parent)
	: QAbstractItemModel(parent),
	  m_server(server) {
		Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server to observe must not be null");
	}


	QModelIndex ServerMimeActionsModel::findMimeType(const QString & mimeType) const {
		return findHelper<MimeTypeColumnIndex>(mimeType);
	}


	QModelIndex ServerMimeActionsModel::findMimeTypeAction(const QString & mimeType) const {
		return findHelper<ActionColumnIndex>(mimeType);
	}


	QModelIndex ServerMimeActionsModel::findMimeTypeCgi(const QString & mimeType) const {
		return findHelper<CgiColumnIndex>(mimeType);
	}


	QModelIndex ServerMimeActionsModel::index(int row, int column, const QModelIndex &) const {
		if(0 > column || 3 <= column) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid column (" << column << ")\n";
			return {};
		}

		if(0 > row) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid row (" << row << ")\n";
			return {};
		}

		// for anything else, return a top-level extension item index
		if(rowCount() <= row) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: row for item index is out of bounds\n";
			return {};
		}

		return createIndex(row, column);
	}


	QModelIndex ServerMimeActionsModel::parent(const QModelIndex &) const {
		return {};
	}


	int ServerMimeActionsModel::rowCount(const QModelIndex &) const {
		return m_server->configuration().registeredMimeTypeCount();
	}


	int ServerMimeActionsModel::columnCount(const QModelIndex &) const {
		return 1 + Equit::max<int, MimeTypeColumnIndex, ActionColumnIndex, CgiColumnIndex>();
	}


	QVariant ServerMimeActionsModel::data(const QModelIndex & index, int role) const {
		if(Qt::DisplayRole != role && Qt::EditRole != role && Qt::DecorationRole != role) {
			return {};
		}

		if(!index.isValid()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: index is not valid\n";
			return {};
		}

		auto row = index.row();

		if(0 > row || rowCount() <= row) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: index is not valid\n";
			return {};
		}

		const auto & config = m_server->configuration();
		const auto mimeTypes = config.registeredMimeTypes();
		const auto & mimeType = mimeTypes[static_cast<std::size_t>(row)];

		switch(index.column()) {
			case MimeTypeColumnIndex:
				if(Qt::DecorationRole == role) {
					return mimeIcon(mimeType);
				}

				return mimeTypes[static_cast<std::size_t>(row)];
				break;

			case ActionColumnIndex:
				switch(role) {
					case Qt::DecorationRole:
						return {};

					case Qt::DisplayRole:
						switch(config.mimeTypeAction(mimeType)) {
							case Configuration::WebServerAction::Ignore:
								return tr("Ignore");

							case Configuration::WebServerAction::Serve:
								return tr("Serve");

							case Configuration::WebServerAction::CGI:
								return tr("CGI");

							case Configuration::WebServerAction::Forbid:
								return tr("Forbid");
						}

					case Qt::EditRole:
						return QVariant::fromValue(config.mimeTypeAction(mimeType));
				}
				break;

			case CgiColumnIndex: {
				if(Configuration::WebServerAction::CGI == config.mimeTypeAction(mimeType)) {
					return config.mimeTypeCgi(mimeType);
				}
				break;
			}
		}

		return {};
	}


	Qt::ItemFlags ServerMimeActionsModel::flags(const QModelIndex & index) const {
		auto ret = QAbstractItemModel::flags(index);

		if(!index.isValid()) {
			return ret;
		}

		ret |= Qt::ItemNeverHasChildren;

		switch(index.column()) {
			case ActionColumnIndex:
				ret |= Qt::ItemIsEditable;
				break;

			case CgiColumnIndex: {
				const auto & config = m_server->configuration();
				const auto mimeTypes = config.registeredMimeTypes();
				const auto row = index.row();

				if(row >= 0 && row < static_cast<int>(mimeTypes.size()) && Configuration::WebServerAction::CGI == config.mimeTypeAction(mimeTypes[static_cast<std::size_t>(row)])) {
					ret |= Qt::ItemIsEditable;
				}

				break;
			}
		}

		return ret;
	}


	QVariant ServerMimeActionsModel::headerData(int section, Qt::Orientation orientation, int role) const {
		if(Qt::DisplayRole != role) {
			return QAbstractItemModel::headerData(section, orientation, role);
		}

		switch(section) {
			case MimeTypeColumnIndex:
				return tr("MIME type");

			case ActionColumnIndex:
				return tr("Action");

			case CgiColumnIndex:
				return tr("CGI executable");
		}

		return {};
	}


	bool ServerMimeActionsModel::setData(const QModelIndex & index, const QVariant & value, int role) {
		if(!index.isValid()) {
			return false;
		}

		if(Qt::EditRole != role) {
			return QAbstractItemModel::setData(index, value, role);
		}

		auto row = index.row();

		if(0 > row || rowCount() <= row) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __FILE__ << "]: invalid index - row does not exist\n";
			return false;
		}

		auto & config = m_server->configuration();

		switch(index.column()) {
			case MimeTypeColumnIndex:
				std::cerr << __PRETTY_FUNCTION__ << " [" << __FILE__ << "]: can't set the MIME type for an action\n";
				return false;

			case ActionColumnIndex: {
				const auto mimeType = config.registeredMimeTypes()[static_cast<std::size_t>(row)];
				const auto oldAction = config.mimeTypeAction(mimeType);
				//				const auto action = Configuration::toWebserverAction(value.value<int>());
				const auto action = value.value<Configuration::WebServerAction>();

				if(action == oldAction) {
					// no change
					return true;
				}

				if(!config.setMimeTypeAction(mimeType, action)) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to set action for \"" << qPrintable(mimeType) << "\"\n";
					return false;
				}

				Q_EMIT actionChanged(mimeType, action);
				return true;
			}

			case CgiColumnIndex: {
				const auto mimeType = config.registeredMimeTypes()[static_cast<std::size_t>(row)];
				const auto oldCgi = config.mimeTypeCgi(mimeType);
				const auto cgi = value.value<QString>();

				if(oldCgi == cgi) {
					// no change
					return true;
				}

				config.setMimeTypeCgi(mimeType, cgi);
				Q_EMIT cgiChanged(mimeType, cgi);
				return true;
			}
		}

		return QAbstractItemModel::setData(index, value, role);
	}


	QModelIndex ServerMimeActionsModel::addMimeType(QString mimeType, Configuration::WebServerAction action, const QString & cgi) {
		auto & config = m_server->configuration();

		if(mimeType.isEmpty()) {
			mimeType = tr("application/x-subtype");

			if(config.mimeTypeIsRegistered(mimeType)) {
				int idx = 1;

				do {
					++idx;
					mimeType = QStringLiteral("application/x-subtype-%1").arg(idx);
				} while(config.mimeTypeIsRegistered(mimeType));
			}
		}
		else if(config.mimeTypeIsRegistered(mimeType)) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: MIME type \"" << qPrintable(mimeType) << "\" already exists\n";
			return {};
		}

		if(!config.setMimeTypeAction(mimeType, action)) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to set action for MIME type \"" << qPrintable(mimeType) << "\"\n";
			return {};
		}

		if(Configuration::WebServerAction::CGI == action) {
			config.setMimeTypeCgi(mimeType, cgi);
		}
		else if(!cgi.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: received CGI \"" << qPrintable(cgi) << "\" for MIME type \"" << qPrintable(mimeType) << "\" but its action was not WebServerAction::CGI\n";
		}

		beginResetModel();
		endResetModel();
		return findMimeTypeAction(mimeType);
	}


	bool ServerMimeActionsModel::removeRows(int row, int count, const QModelIndex & parent) {
		if(1 > count) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: count of items to remove must be > 01\n";
			return false;
		}

		auto & config = m_server->configuration();
		const int mimeTypeCount = config.registeredMimeTypeCount();

		if(0 > row || mimeTypeCount <= row) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: first row to remove out of bounds: " << row << "\n";
			return false;
		}

		int endRow = row + count - 1;

		if(mimeTypeCount <= endRow) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: last row to remove out of bounds: " << endRow << "\n";
			return false;
		}

		beginRemoveRows(parent, row, endRow);
		const auto mimeTypes = config.registeredMimeTypes();
		auto begin = mimeTypes.cbegin() + row;

		std::for_each(begin, begin + count, [&config](const auto & mimeType) {
			std::cout << "calling unsetMimeTypeAction(\"" << qPrintable(mimeType) << "\")\n"
						 << std::flush;
			config.unsetMimeTypeAction(mimeType);
		});

		endRemoveRows();
		return true;
	}


}  // namespace EquitWebServer