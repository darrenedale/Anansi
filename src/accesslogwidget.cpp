#include "src/accesslogwidget.h"

#include <iostream>


namespace EquitWebServer {


	AccessLogWidget::AccessLogWidget(QWidget * parent)
	: QTreeWidget(parent) {
		QTreeWidgetItem * accessLogHeader = new QTreeWidgetItem;
		accessLogHeader->setText(0, tr("Remote IP"));
		accessLogHeader->setText(1, tr("Remote Port"));
		accessLogHeader->setText(2, tr("Resource Requested"));
		accessLogHeader->setText(3, tr("Response/Action"));
		setHeaderItem(accessLogHeader);
		setRootIsDecorated(false);
	}


	void AccessLogWidget::addPolicyEntry(QString addr, uint16_t port, Configuration::ConnectionPolicy policy) {
		addTopLevelItem(new AccessLogTreeItem(addr, port, policy));
	}


	void AccessLogWidget::addActionEntry(QString addr, uint16_t port, QString resource, Configuration::WebServerAction action) {
		addTopLevelItem(new AccessLogTreeItem(addr, port, resource, action));
	}


}  // namespace EquitWebServer
