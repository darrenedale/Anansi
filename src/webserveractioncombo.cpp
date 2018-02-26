#include "webserveractioncombo.h"
#include "connectionpolicycombo.h"

#include <QVariant>
#include <QIcon>

#include "types.h"


Q_DECLARE_METATYPE(EquitWebServer::WebServerAction)


namespace EquitWebServer {


	WebServerActionCombo::WebServerActionCombo(QWidget * parent)
	: QComboBox(parent) {
		// TODO add icons
		QComboBox::addItem(tr("Ignore"), QVariant::fromValue(WebServerAction::Ignore));
		QComboBox::addItem(QIcon::fromTheme("dialog-ok"), tr("Serve"), QVariant::fromValue(WebServerAction::Serve));
		QComboBox::addItem(QIcon::fromTheme("system-run"), tr("CGI"), QVariant::fromValue(WebServerAction::CGI));
		QComboBox::addItem(QIcon::fromTheme("cards-block"), tr("Forbid"), QVariant::fromValue(WebServerAction::Forbid));
		setToolTip(tr("<p>Choose what to do with requests of this type.</p>"));

		connect(this, qOverload<int>(&QComboBox::currentIndexChanged), [this](int) {
			Q_EMIT webServerActionChanged(webServerAction());
		});
	}


	WebServerAction WebServerActionCombo::webServerAction() {
		return currentData().value<WebServerAction>();
	}


	void WebServerActionCombo::setWebServerAction(WebServerAction action) {
		setCurrentIndex(findData(QVariant::fromValue(action)));
	}


}  // namespace EquitWebServer
