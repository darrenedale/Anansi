#include "iplineeditaction.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>


namespace EquitWebServer {


	IpLineEditAction::IpLineEditAction(QObject * parent)
	: QWidgetAction(parent) {
		auto * container = new QWidget;
		m_ipAddress = new QLineEdit();
		auto * add = new QPushButton(QIcon::fromTheme("list-add", QIcon(":/icons/buttons/addtolist")), {});
		QHBoxLayout * layout = new QHBoxLayout;
		layout->addWidget(new QLabel(tr("IP address")));
		layout->addWidget(m_ipAddress);
		layout->addWidget(add);
		container->setLayout(layout);

		connect(add, &QPushButton::clicked, [this]() {
			Q_EMIT addIpAddressClicked(m_ipAddress->text());
		});

		setDefaultWidget(container);
	}


	IpLineEditAction::~IpLineEditAction() = default;


	QString IpLineEditAction::ipAddress() const {
		return m_ipAddress->text();
	}


	void IpLineEditAction::setIpAddress(const QString & addr) {
		m_ipAddress->setText(addr);
	}


}  // namespace EquitWebServer
