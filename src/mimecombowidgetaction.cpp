#include "mimecombowidgetaction.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "mimecombo.h"

namespace EquitWebServer {


	MimeComboWidgetAction::MimeComboWidgetAction(QObject * parent)
	: QWidgetAction(parent) {
		auto * container = new QWidget;
		m_combo = new MimeCombo(true);
		auto * add = new QPushButton(QIcon::fromTheme("dialog-ok-accept"), {});
		QHBoxLayout * layout = new QHBoxLayout;
		layout->addWidget(new QLabel(tr("Mime type")));
		layout->addWidget(m_combo);
		layout->addWidget(add);
		container->setLayout(layout);

		connect(add, &QPushButton::clicked, [this]() {
			Q_EMIT addMimeTypeClicked(m_combo->currentMimeType());
		});

		setDefaultWidget(container);
	}


	void MimeComboWidgetAction::setMimeTypes(std::vector<QString> mimeTypes) {
		m_combo->clear();

		for(const auto & mimeType : mimeTypes) {
			m_combo->addMimeType(mimeType);
		}
	}


	void MimeComboWidgetAction::addMimeType(const QString & mimeType) {
		m_combo->addMimeType(mimeType);
	}


	MimeComboWidgetAction::~MimeComboWidgetAction() = default;


}  // namespace EquitWebServer
