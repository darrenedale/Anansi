#include "serverfileassociationsmodel.h"

#include <iostream>

#include "server.h"
#include "mimeicons.h"


namespace EquitWebServer {


	ServerFileAssociationsModel::ServerFileAssociationsModel(Server * server, QObject * parent)
	: QAbstractItemModel(parent),
	  m_server(server) {
		Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server to observe must not be null");
	}


	QModelIndex ServerFileAssociationsModel::index(int row, int column, const QModelIndex & parent) const {
		if(0 != column) {
			std::cerr << __PRETTY_FUNCTION__ << ": invalid column (" << column << ")\n";
			return {};
		}

		if(0 > row) {
			std::cerr << __PRETTY_FUNCTION__ << ": invalid row (" << column << ")\n";
			return {};
		}

		if(parent.isValid()) {
			if(0 == parent.internalId()) {
				// extension items have associated MIME types as children
				const auto mimeTypes = m_server->configuration().mimeTypesForFileExtension(parent.data().value<QString>());

				if(mimeTypes.size() <= static_cast<std::size_t>(row)) {
					std::cerr << __PRETTY_FUNCTION__ << ": row for MIME type item index is out of bounds\n";
					return {};
				}

				// ID is parent row + 1. this leaves ID = 0 to be used as an indicator
				// that an index is an extension item index. the ID for MIME type items
				// has 1 subtracted to find the row index of its parent extension item
				return createIndex(row, column, static_cast<quintptr>(parent.row() + 1));
			}

			// if parent's ID is > 0, it's a MIME type item, which has no children, so just return
			// and invalid index
			std::cerr << __PRETTY_FUNCTION__ << ": parent index does not have any children\n";
			return {};
		}

		// for anything else, return a top-level extension item index
		if(rowCount() <= row) {
			std::cerr << __PRETTY_FUNCTION__ << ": row for extension item index is out of bounds\n";
			return {};
		}

		return createIndex(row, column, static_cast<quintptr>(0));
	}


	QModelIndex ServerFileAssociationsModel::parent(const QModelIndex & index) const {
		if(!index.isValid()) {
			std::cerr << __PRETTY_FUNCTION__ << ": invalid index == invalid parent\n";
			return {};
		}

		if(0 == index.internalId()) {
			// this is an extension item index and as such it has no parent
			return {};
		}

		return createIndex(static_cast<int>(index.internalId() - 1), 0, static_cast<quintptr>(0));
	}


	int ServerFileAssociationsModel::rowCount(const QModelIndex & parent) const {
		if(parent.isValid()) {
			if(0 < parent.internalId()) {
				// MIME type items don't have children
				return 0;
			}

			const auto ext = parent.data().value<QString>();
			return static_cast<int>(m_server->configuration().mimeTypesForFileExtension(ext).size());
		}

		return m_server->configuration().registeredFileExtensions().size();
	}


	int ServerFileAssociationsModel::columnCount(const QModelIndex &) const {
		return 1;
	}


	QVariant ServerFileAssociationsModel::data(const QModelIndex & index, int role) const {
		if(Qt::DisplayRole != role && Qt::EditRole != role && Qt::DecorationRole != role) {
			return {};
		}

		if(!index.isValid()) {
			std::cerr << __PRETTY_FUNCTION__ << ": index is not valid\n";
			return {};
		}

		if(0 != index.column()) {
			std::cerr << __PRETTY_FUNCTION__ << ": index column must be 0\n";
			return {};
		}

		const auto extensions = m_server->configuration().registeredFileExtensions();

		if(0 == index.internalId()) {
			if(Qt::DecorationRole == role) {
				return {};
			}

			int idx = index.row();

			if(0 > idx || extensions.size() <= idx) {
				std::cerr << __PRETTY_FUNCTION__ << ": extensions index row is not valid\n";
				return {};
			}

			return extensions[idx];
		}

		int idx = static_cast<int>(index.internalId() - 1);

		if(0 > idx || extensions.size() <= idx) {
			std::cerr << __PRETTY_FUNCTION__ << ": invalid parent row index\n";
			return {};
		}

		auto ext = extensions[idx];

		if(ext.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << ": empty extension when looking up MIME type index\n";
			return {};
		}

		auto mimeTypes = m_server->configuration().mimeTypesForFileExtension(ext);
		idx = index.row();

		if(0 > idx || static_cast<int>(mimeTypes.size()) <= idx) {
			std::cerr << __PRETTY_FUNCTION__ << ": MIME type index row is not valid\n";
			return {};
		}

		if(Qt::DecorationRole == role) {
			return mimeIcon(mimeTypes[static_cast<std::size_t>(idx)]);
		}

		return mimeTypes[static_cast<std::size_t>(idx)];
	}


	Qt::ItemFlags ServerFileAssociationsModel::flags(const QModelIndex & index) const {
		auto ret = QAbstractItemModel::flags(index);

		if(index.isValid()) {
			ret |= Qt::ItemIsEditable;

			if(index.parent().isValid()) {
				// it's a MIME type item, which never has any children
				ret |= Qt::ItemNeverHasChildren;
			}
		}

		return ret;
	}


	QVariant ServerFileAssociationsModel::headerData(int section, Qt::Orientation orientation, int role) const {
		if(Qt::DisplayRole != role) {
			return QAbstractItemModel::headerData(section, orientation, role);
		}

		if(0 == section && Qt::Horizontal == orientation) {
			return tr("MIME Type Associations");
		}

		return {};
	}


	bool ServerFileAssociationsModel::setData(const QModelIndex & index, const QVariant & value, int role) {
		if(!index.isValid()) {
			return false;
		}

		if(Qt::EditRole != role) {
			return QAbstractItemModel::setData(index, value, role);
		}

		const auto parent = index.parent();

		if(parent.isValid()) {
			// it's a MIME type item
			const auto ext = parent.data().value<QString>();

			if(ext.isEmpty()) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: empty extension when attempting to edit MIME type value\n";
				return false;
			}

			const auto mimeType = value.value<QString>().toLower();
			// TODO check MIME type is valid

			if(mimeType.isEmpty()) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: empty MIME type value\n";
				return false;
			}

			auto & config = m_server->configuration();

			if(!config.fileExtensionIsRegistered(ext)) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: extension \"" << qPrintable(ext) << "\" is not present in config\n";
				return false;
			}

			const auto oldMimeType = index.data().value<QString>();

			if(oldMimeType == mimeType) {
				return true;
			}

			config.removeFileExtensionMimeType(ext, oldMimeType);
			config.addFileExtensionMimeType(ext, mimeType);
			Q_EMIT dataChanged(index, index, QVector<int>() << Qt::DisplayRole << Qt::EditRole);
		}
	}


}  // namespace EquitWebServer
