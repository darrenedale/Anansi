#ifndef CONNECTIONPOLICYCOMBO_H
#define CONNECTIONPOLICYCOMBO_H

#include <QComboBox>

#include "configuration.h"

namespace EquitWebServer {

	class ConnectionPolicyCombo : public QComboBox {
		Q_OBJECT

	public:
		explicit ConnectionPolicyCombo(QWidget * parent = nullptr);
		virtual ~ConnectionPolicyCombo() = default;

		void addItem() = delete;

		Configuration::ConnectionPolicy connectionPolicy();

	public Q_SLOTS:
		void setConnectionPolicy(Configuration::ConnectionPolicy polilcy);

	Q_SIGNALS:
		void connectionPolicyChanged(Configuration::ConnectionPolicy);
	};

}  // namespace EquitWebServer

#endif  // CONNECTIONPOLICYCOMBO_H
