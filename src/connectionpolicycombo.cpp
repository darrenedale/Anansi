#include "connectionpolicycombo.h"

#include "configuration.h"


Q_DECLARE_METATYPE(EquitWebServer::ConnectionPolicy)


namespace EquitWebServer {


	ConnectionPolicyCombo::ConnectionPolicyCombo(QWidget * parent)
	: QComboBox(parent) {
		QComboBox::addItem(QIcon(":/icons/connectionpolicies/nopolicy"), tr("No Policy"), QVariant::fromValue(ConnectionPolicy::None));
		QComboBox::addItem(QIcon::fromTheme("dialog-ok-apply", QIcon(":/icons/connectionpolicies/accept")), tr("Accept Connection"), QVariant::fromValue(ConnectionPolicy::Accept));
		QComboBox::addItem(QIcon::fromTheme("dialog-cancel", QIcon(":/icons/connectionpolicies/reject")), tr("Reject Connection"), QVariant::fromValue(ConnectionPolicy::Reject));
		setToolTip(tr("<p>Choose the policy to use for HTTP connections from IP addresses that do not have a specific policy, including those for which <strong>No Policy</strong> has been chosen.</p>"));

		connect(this, qOverload<int>(&QComboBox::currentIndexChanged), [this](int) {
			Q_EMIT connectionPolicyChanged(connectionPolicy());
		});
	}


	ConnectionPolicy ConnectionPolicyCombo::connectionPolicy() {
		return currentData().value<ConnectionPolicy>();
	}


	void ConnectionPolicyCombo::setConnectionPolicy(ConnectionPolicy policy) {
		setCurrentIndex(findData(QVariant::fromValue(policy)));
	}


}  // namespace EquitWebServer
