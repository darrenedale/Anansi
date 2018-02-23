#include "mimetypecomboaction.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "mimetypecombo.h"

namespace EquitWebServer {


	MimeTypeComboAction::MimeTypeComboAction(QObject * parent)
	: QWidgetAction(parent) {
		auto * container = new QWidget;
		m_combo = new MimeTypeCombo(true);
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


	void MimeTypeComboAction::setMimeTypes(std::vector<QString> mimeTypes) {
		m_combo->clear();

		for(const auto & mimeType : mimeTypes) {
			m_combo->addMimeType(mimeType);
		}
	}


	void MimeTypeComboAction::addMimeType(const QString & mimeType) {
		m_combo->addMimeType(mimeType);
	}


	MimeTypeComboAction::~MimeTypeComboAction() = default;


}  // namespace EquitWebServer
