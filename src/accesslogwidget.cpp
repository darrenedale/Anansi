#include "src/accesslogwidget.h"

#include <iostream>


Q_DECLARE_METATYPE(EquitWebServer::ConnectionPolicy);
Q_DECLARE_METATYPE(EquitWebServer::WebServerAction);


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


	void AccessLogWidget::addPolicyEntry(QString addr, quint16 port, ConnectionPolicy policy) {
		addTopLevelItem(new AccessLogTreeItem(addr, port, policy));
	}


	void AccessLogWidget::addActionEntry(QString addr, quint16 port, QString resource, WebServerAction action) {
		addTopLevelItem(new AccessLogTreeItem(addr, port, resource, action));
	}


}  // namespace EquitWebServer
