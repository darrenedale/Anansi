/** \file IpListWidget.h
  * \author Darren Edale
  * \version 0.9.9
  * \date 19th June, 2012
  *
  * \brief Definition of the bpIpListWidget class for EquitWebServer
  *
  * \todo
  * - class documentation.
  * - decide on application license.
  *
  * \par Changes
  * - (2012-06-19) file documentation created.
  *
  */

#ifndef EQUITWEBSERVER_IPLISTWIDGET_H
#define EQUITWEBSERVER_IPLISTWIDGET_H

#include <QTreeWidget>

class QContextMenuEvent;

namespace EquitWebServer {
	class IpListWidget : public QTreeWidget {
		Q_OBJECT

	public:
		explicit IpListWidget(QWidget * parent = nullptr);
		void setSelectionMode(SelectionMode) = delete;
		void insertTopLevelItem(int index, QTreeWidgetItem * item);

	protected:
		void contextMenuEvent(QContextMenuEvent * event);

	protected Q_SLOTS:
		void removeIPAddress(int i);
		void removeSelectedIPAddress();

	Q_SIGNALS:
		void ipAddressRemoved(const QString &);
	};
}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_IPLISTWIDGET_H
