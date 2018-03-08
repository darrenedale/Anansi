/// \file accesslogwidget.h
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Declaration of the AccessLogWidget class for Anansi.
///
/// \dep
/// - <memory>
/// - <QWidget>
/// - <QDateTime>
/// - types.h
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_ACCESSLOGWIDGET_H
#define ANANSI_ACCESSLOGWIDGET_H

#include <memory>

#include <QWidget>
#include <QDateTime>

#include "types.h"

class QString;

namespace Anansi {

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

		void addPolicyEntry(const QDateTime & timestamp, const QString & addr, uint16_t port, ConnectionPolicy policy);
		void addActionEntry(const QDateTime & timestamp, const QString & addr, uint16_t port, QString resource, WebServerAction action);

		inline void addPolicyEntry(const QString & addr, uint16_t port, ConnectionPolicy policy) {
			addPolicyEntry(QDateTime::currentDateTime(), addr, port, policy);
		}

		inline void addActionEntry(const QString & addr, uint16_t port, QString resource, WebServerAction action) {
			addActionEntry(QDateTime::currentDateTime(), addr, port, resource, action);
		}

	private:
		std::unique_ptr<Ui::AccessLogWidget> m_ui;
	};

}  // namespace Anansi

#endif  // ANANSI_ACCESSLOGWIDGET_H
