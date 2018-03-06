/// \file accesslogwidget.h
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Declaration of the AccessLogWidget class for EquitWebServer
///
/// \par Changes
/// - (2018-03) First release.

#ifndef EQUITWEBSERVER_ACCESSLOGWIDGET_H
#define EQUITWEBSERVER_ACCESSLOGWIDGET_H

#include <memory>

#include <QWidget>

#include "types.h"

class QString;

namespace EquitWebServer {

	namespace Ui {
		class AccessLogWidget;
	}

	class AccessLogWidget : public QWidget {
		Q_OBJECT

	public:
		explicit AccessLogWidget(QWidget * parent = nullptr);
		virtual ~AccessLogWidget();

	public Q_SLOTS:
		void save();
		void saveAs(const QString & fileName);

		void clear();

		// TODO timestamp entries
		void addPolicyEntry(const QString & addr, uint16_t port, ConnectionPolicy policy);
		void addActionEntry(const QString & addr, uint16_t port, QString resource, WebServerAction action);

	private:
		std::unique_ptr<Ui::AccessLogWidget> m_ui;
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_ACCESSLOGWIDGET_H
