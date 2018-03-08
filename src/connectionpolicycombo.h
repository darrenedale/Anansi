/// \file connectionpolicycombo.h
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Declaration of the ConnectionPolicyCombo class for Anansi.
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_CONNECTIONPOLICYCOMBO_H
#define ANANSI_CONNECTIONPOLICYCOMBO_H

#include <QComboBox>

#include "types.h"

namespace Anansi {

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

}  // namespace Anansi

#endif  // ANANSI_CONNECTIONPOLICYCOMBO_H
