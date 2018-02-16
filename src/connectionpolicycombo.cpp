#include "connectionpolicycombo.h"

#include "configuration.h"


Q_DECLARE_METATYPE(EquitWebServer::Configuration::ConnectionPolicy)


namespace EquitWebServer {


	ConnectionPolicyCombo::ConnectionPolicyCombo(QWidget * parent)
	: QComboBox(parent) {
		QComboBox::addItem(QIcon(":/icons/connectionpolicies/nopolicy"), tr("No Policy"), QVariant::fromValue(Configuration::ConnectionPolicy::None));
		QComboBox::addItem(QIcon::fromTheme("dialog-ok-apply", QIcon(":/icons/connectionpolicies/accept")), tr("Accept Connection"), QVariant::fromValue(Configuration::ConnectionPolicy::Accept));
		QComboBox::addItem(QIcon::fromTheme("dialog-cancel", QIcon(":/icons/connectionpolicies/reject")), tr("Reject Connection"), QVariant::fromValue(Configuration::ConnectionPolicy::Reject));
		setToolTip(tr("<p>Choose the policy to use for HTTP connections from IP addresses that do not have a specific policy, including those for which <strong>No Policy</strong> has been chosen.</p>"));

		connect(this, qOverload<int>(&QComboBox::currentIndexChanged), [this](int) {
			Q_EMIT connectionPolicyChanged(connectionPolicy());
		});
	}


	Configuration::ConnectionPolicy ConnectionPolicyCombo::connectionPolicy() {
		return currentData().value<Configuration::ConnectionPolicy>();
	}


	void ConnectionPolicyCombo::setConnectionPolicy(Configuration::ConnectionPolicy policy) {
		setCurrentIndex(findData(QVariant::fromValue(policy)));
	}
}  // namespace EquitWebServer
