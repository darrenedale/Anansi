#ifndef EQUITWEBSERVER_FILEASSOCIATIONEXTENSIONITEM_H
#define EQUITWEBSERVER_FILEASSOCIATIONEXTENSIONITEM_H

#include <QTreeWidgetItem>

#include "configuration.h"

class QString;

namespace EquitWebServer {

	class FileAssociationExtensionItem : public QTreeWidgetItem {
	public:
		static constexpr const int ItemType = QTreeWidgetItem::UserType + 9002;

		explicit FileAssociationExtensionItem(const QString & ext);

		QString previousExtension() const;
		QString extension() const;
		void setExtension(const QString & ext);

	private:
		void refresh();
	};
}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_FILEASSOCIATIONEXTENSIONITEM_H
