#include "serverfileassociationsmodel.h"

#include "server.h"


namespace EquitWebServer {


	ServerFileAssociationsModel::ServerFileAssociationsModel(Server * server, QObject * parent)
	: QAbstractItemModel(parent),
	  m_server(server) {
		Q_ASSERT_X(m_server, __PRETTY_FUNCTION__, "server to observe must not be null");
	}


	QModelIndex ServerFileAssociationsModel::index(int row, int column, const QModelIndex & parent) const {
		if(0 != column) {
			return {};
		}

		if(0 > row) {
			return {};
		}

		if(parent.isValid()) {
			if(0 == parent.internalId()) {
				// extension items have associated MIME types as children
				const auto mimeTypes = m_server->configuration().mimeTypesForFileExtension(parent.data().value<QString>());

				if(mimeTypes.size() <= static_cast<std::size_t>(row)) {
					return {};
				}

				// ID is parent row plus one. this leaves ID = 0 to be used as an indicator
				// that an index is an extension item index. the ID for MIME type items
				// has 1 subtracted to find the row index of its parent extension item
				return createIndex(row, column, static_cast<quintptr>(parent.row() + 1));
			}

			// if parent's ID is > 0, it's a MIME type item, which has no children, so just return
			// and invalid index
			return {};
		}

		// for anything else, return a top-level extension item index
		if(rowCount() <= row) {
			return {};
		}

		return createIndex(row, column, static_cast<quintptr>(0));
	}


	QModelIndex ServerFileAssociationsModel::parent(const QModelIndex & index) const {
		if(!index.isValid()) {
			return {};
		}

		if(0 == index.internalId()) {
			// this is an extension item index and as such it has no parent
			return {};
		}

		return createIndex(static_cast<int>(index.internalId()) - 1, 0, static_cast<quintptr>(0));
	}


	int ServerFileAssociationsModel::rowCount(const QModelIndex & parent) const {
		if(parent.isValid()) {
			if(0 < parent.internalId()) {
				// MIME type items don't have children
				return 0;
			}

			return static_cast<int>(m_server->configuration().mimeTypesForFileExtension(parent.data().value<QString>()).size());
		}

		return m_server->configuration().registeredFileExtensions().size();
	}


	int ServerFileAssociationsModel::columnCount(const QModelIndex &) const {
		return 1;
	}


	QVariant ServerFileAssociationsModel::data(const QModelIndex & index, int) const {
		if(!index.isValid()) {
			return {};
		}

		if(0 != index.column()) {
			return {};
		}

		const auto extensions = m_server->configuration().registeredFileExtensions();

		if(0 == index.internalId()) {
			int idx = index.row();

			if(0 > idx || extensions.size() <= idx) {
				return {};
			}

			return extensions[idx];
		}

		int idx = static_cast<int>(index.internalId() - 1);

		if(0 > idx || extensions.size() <= idx) {
			return {};
		}

		auto ext = extensions[idx];

		if(ext.isEmpty()) {
			return {};
		}

		auto mimeTypes = m_server->configuration().mimeTypesForFileExtension(ext);
		idx = index.row();

		if(0 > idx || static_cast<int>(mimeTypes.size()) <= idx) {
			return {};
		}

		return mimeTypes[static_cast<std::size_t>(idx)];
	}


}  // namespace EquitWebServer
