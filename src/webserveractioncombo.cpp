#include "webserveractioncombo.h"
#include "connectionpolicycombo.h"

#include "configuration.h"


Q_DECLARE_METATYPE(EquitWebServer::Configuration::WebServerAction)


namespace EquitWebServer {


	WebServerActionCombo::WebServerActionCombo(QWidget * parent)
	: QComboBox(parent) {
		// TODO add icons
		QComboBox::addItem(tr("Ignore"), QVariant::fromValue(Configuration::WebServerAction::Ignore));
		QComboBox::addItem(tr("Serve"), QVariant::fromValue(Configuration::WebServerAction::Serve));
		QComboBox::addItem(tr("CGI"), QVariant::fromValue(Configuration::WebServerAction::CGI));
		QComboBox::addItem(tr("Forbid"), QVariant::fromValue(Configuration::WebServerAction::Forbid));
		setToolTip(tr("<p>Choose what to do with requests of this type.</p>"));

		connect(this, qOverload<int>(&QComboBox::currentIndexChanged), [this](int) {
			Q_EMIT webServerActionChanged(webServerAction());
		});
	}


	Configuration::WebServerAction WebServerActionCombo::webServerAction() {
		return currentData().value<Configuration::WebServerAction>();
	}


	void WebServerActionCombo::setWebServerAction(Configuration::WebServerAction action) {
		setCurrentIndex(findData(QVariant::fromValue(action)));
	}


}  // namespace EquitWebServer
