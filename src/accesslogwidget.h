/// \file accesslogwidget.h
/// \author Darren Edale
/// \version 0.9.9
/// \date February, 2018
///
/// \brief Declaration of the AccessLogWidget class for EquitWebServer
///
/// \par Changes
/// - (2018-02) First release.

#ifndef EQUITWEBSERVER_ACCESSLOGWIDGET_H
#define EQUITWEBSERVER_ACCESSLOGWIDGET_H

#include <QTreeWidget>

#include "accesslogtreeitem.h"
#include "configuration.h"

namespace EquitWebServer {

	class AccessLogTreeItem;

	class AccessLogWidget : public QTreeWidget {
		Q_OBJECT

	public:
		explicit AccessLogWidget(QWidget * parent = nullptr);


		// ensure only items of required type can be (easily) added
		inline void insertTopLevelItem(int idx, AccessLogTreeItem * item) {
			QTreeWidget::insertTopLevelItem(idx, item);
		}

		inline void addTopLevelItem(AccessLogTreeItem * item) {
			QTreeWidget::addTopLevelItem(item);
		}

	public Q_SLOTS:
		void addPolicyEntry(QString addr, quint16 port, ConnectionPolicy policy);
		void addActionEntry(QString addr, quint16 port, QString resource, WebServerAction action);
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_ACCESSLOGWIDGET_H
