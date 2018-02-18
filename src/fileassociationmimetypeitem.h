#ifndef EQUITWEBSERVER_FILEASSOCIATIONMIMETYPEITEM_H
#define EQUITWEBSERVER_FILEASSOCIATIONMIMETYPEITEM_H

#include <QTreeWidgetItem>

class QString;

namespace EquitWebServer {

	class FileAssociationMimeTypeItem : public QTreeWidgetItem {
	public:
		static constexpr const int ItemType = QTreeWidgetItem::UserType + 9001;

		explicit FileAssociationMimeTypeItem(const QString & mimeType);

		QString previousMimeType() const;
		QString mimeType() const;
		void setMimeType(const QString & mimeType);

	private:
		void refresh();
	};
}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_FILEASSOCIATIONMIMETYPEITEM_H
