#ifndef ACCESSLOGWIDGET_H
#define ACCESSLOGWIDGET_H

#include <QTreeWidget>

#include "accesslogtreeitem.h"
#include "configuration.h"

namespace EquitWebServer {

	class AccessLogTreeItem;

	class AccessLogWidget : public QTreeWidget {
		Q_OBJECT

	public:
		AccessLogWidget(QWidget * parent = nullptr);

		inline void insertTopLevelItem(int idx, AccessLogTreeItem * item) {
			QTreeWidget::insertTopLevelItem(idx, item);
		}

		inline void addTopLevelItem(AccessLogTreeItem * item) {
			QTreeWidget::addTopLevelItem(item);
		}

	public Q_SLOTS:
		void addPolicyEntry(QString addr, uint16_t port, Configuration::ConnectionPolicy policy);
		void addActionEntry(QString addr, uint16_t port, QString resource, Configuration::WebServerAction action);
	};

}  // namespace EquitWebServer

#endif  // ACCESSLOGWIDGET_H
