/// \file connectionpolicycombo.h
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Declaration of the ConnectionPolicyCombo class for EquitWebServer
///
/// \par Changes
/// - (2018-03) First release.

#ifndef EQUITWEBSERVER_CONNECTIONPOLICYCOMBO_H
#define EQUITWEBSERVER_CONNECTIONPOLICYCOMBO_H

#include <QComboBox>

#include "types.h"

namespace EquitWebServer {

	class ConnectionPolicyCombo : public QComboBox {
		Q_OBJECT

	public:
		explicit ConnectionPolicyCombo(QWidget * parent = nullptr);
		virtual ~ConnectionPolicyCombo() = default;

		void addItem() = delete;

		ConnectionPolicy connectionPolicy();

	public Q_SLOTS:
		void setConnectionPolicy(ConnectionPolicy polilcy);

	Q_SIGNALS:
		void connectionPolicyChanged(ConnectionPolicy);
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_CONNECTIONPOLICYCOMBO_H
