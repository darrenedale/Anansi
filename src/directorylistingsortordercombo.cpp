#include "directorylistingsortordercombo.h"

#include <QVariant>
#include <QIcon>

#include "types.h"


Q_DECLARE_METATYPE(EquitWebServer::DirectoryListingSortOrder)


namespace EquitWebServer {


	DirectoryListingSortOrderCombo::DirectoryListingSortOrderCombo(QWidget * parent)
	: QComboBox(parent) {
		QComboBox::addItem(QIcon::fromTheme("view-sort-ascending"), tr("Ascending"), QVariant::fromValue(DirectoryListingSortOrder::Ascending));
		QComboBox::addItem(QIcon::fromTheme("view-sort-ascending"), tr("Ascending, directories first"), QVariant::fromValue(DirectoryListingSortOrder::AscendingDirectoriesFirst));
		QComboBox::addItem(QIcon::fromTheme("view-sort-ascending"), tr("Ascending, files first"), QVariant::fromValue(DirectoryListingSortOrder::AscendingFilesFirst));
		QComboBox::addItem(QIcon::fromTheme("view-sort-descending"), tr("Descending"), QVariant::fromValue(DirectoryListingSortOrder::Descending));
		QComboBox::addItem(QIcon::fromTheme("view-sort-descending"), tr("Descending, directories first"), QVariant::fromValue(DirectoryListingSortOrder::DescendingDirectoriesFirst));
		QComboBox::addItem(QIcon::fromTheme("view-sort-descending"), tr("Descending, files first"), QVariant::fromValue(DirectoryListingSortOrder::DescendingFilesFirst));
		setToolTip(tr("<p>Choose how to sort the entries in generated directory listings.</p>"));

		connect(this, qOverload<int>(&QComboBox::currentIndexChanged), [this](int) {
			Q_EMIT sortOrderChanged(sortOrder());
		});
	}


	DirectoryListingSortOrder DirectoryListingSortOrderCombo::sortOrder() {
		return currentData().value<DirectoryListingSortOrder>();
	}


	void DirectoryListingSortOrderCombo::setSortOrder(DirectoryListingSortOrder order) {
		setCurrentIndex(findData(QVariant::fromValue(order)));
	}


}  // namespace EquitWebServer
